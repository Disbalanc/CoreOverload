#include "engine/combat/CombatManager.h"
#include <algorithm>
#include <stdexcept>

namespace engine {

CombatManager::CombatManager(EventBus& eventBus, Random& rng)
    : m_eventBus(eventBus)
    , m_rng(rng)
{
}

// ══════════════════════════════════════════════════════════
// ИНИЦИАЛИЗАЦИЯ БОЯ
// ══════════════════════════════════════════════════════════

void CombatManager::startCombat(Player& player,
                                 const std::vector<EnemyConfig>& enemyConfigs)
{
    m_player = &player;
    m_enemies.clear();
    m_turnNumber = 0;
    m_playerWon = false;

    // Создаём врагов по конфигурациям
    for (const auto& config : enemyConfigs) {
        auto enemy = std::make_unique<Enemy>(config.id, config.name, config.hp);
        enemy->setAI(std::make_unique<PatternAI>(config.pattern));
        m_enemies.push_back(std::move(enemy));
    }

    // Инициализируем колоду игрока для боя
    m_player->getDeck().initForCombat(m_rng);

    // Привязываем колбэк тасовки к EventBus
    m_player->getDeck().setShuffleCallback([this](int count) {
        m_eventBus.publish({EventType::DeckShuffled, {{"count", count}}});
    });

    // Триггерим артефакты OnCombatStart
    for (const auto& artifact : m_player->getArtifacts()) {
        if (artifact->getTrigger() == ArtifactTrigger::OnCombatStart) {
            for (const auto& eff : artifact->getEffects()) {
                switch (eff.type) {
                    case ArtifactEffectType::GainBlock:
                        m_player->gainBlock(eff.value);
                        break;
                    case ArtifactEffectType::HealHp:
                        m_player->heal(eff.value);
                        break;
                    default: break;
                }
            }
        }
    }

    // Публикуем событие для UI
    m_eventBus.publish({EventType::CombatStarted, {
        {"enemy_count", static_cast<int>(m_enemies.size())}
    }});

    // Начинаем первый ход
    startPlayerTurn();
}

// ══════════════════════════════════════════════════════════
// ХОД ИГРОКА
// ══════════════════════════════════════════════════════════

void CombatManager::startPlayerTurn() {
    m_turnNumber++;
    m_phase = TurnPhase::PlayerTurnStart;

    // 1. Сбросить файрвол игрока (он не сохраняется между ходами)
    m_player->resetBlock();

    // 2. Восстановить энергию (Тактовую частоту)
    m_player->resetEnergy();

    // 3. Добрать 5 карт из колоды
    m_player->getDeck().drawCards(5, m_rng);

    // Триггерим артефакты OnTurnStart
    for (const auto& artifact : m_player->getArtifacts()) {
        if (artifact->getTrigger() == ArtifactTrigger::OnTurnStart) {
            for (const auto& eff : artifact->getEffects()) {
                switch (eff.type) {
                    case ArtifactEffectType::GainEnergy:
                        m_player->gainEnergy(eff.value);
                        break;
                    case ArtifactEffectType::DrawCards:
                        m_player->getDeck().drawCards(eff.value, m_rng);
                        break;
                    case ArtifactEffectType::GainBlock:
                        m_player->gainBlock(eff.value);
                        break;
                    default: break;
                }
            }
        }
    }

    // 4. Враги показывают свои намерения на этот ход
    for (auto& enemy : m_enemies) {
        if (enemy->isAlive()) {
            enemy->updateIntent(m_turnNumber);
        }
    }

    // 5. Переходим в фазу действий — ждём кликов игрока
    m_phase = TurnPhase::PlayerAction;

    m_eventBus.publish({EventType::TurnStarted, {
        {"turn", m_turnNumber}
    }});
}

// ══════════════════════════════════════════════════════════
// РОЗЫГРЫШ КАРТЫ
// ══════════════════════════════════════════════════════════

bool CombatManager::playCard(int cardIndex, int targetIndex) {
    // Проверяем, что сейчас ход игрока
    if (m_phase != TurnPhase::PlayerAction) return false;

    const auto& hand = m_player->getDeck().getHand();

    // Проверяем валидность индекса
    if (cardIndex < 0 || cardIndex >= static_cast<int>(hand.size())) {
        return false;
    }

    const Card* card = hand[cardIndex];

    // Проверяем, хватает ли энергии (ЦПУ)
    if (!m_player->useEnergy(card->getCurrentCost())) {
        m_eventBus.publish({EventType::DamageReceived, {
            {"error", std::string("not_enough_energy")}
        }});
        return false; // Не хватает Тактовой частоты!
    }

    // Проверяем цель для карт типа SingleEnemy
    if (card->getTarget() == CardTarget::SingleEnemy) {
        Enemy* target = getAliveEnemy(targetIndex);
        if (!target) {
            // Возвращаем энергию — цель невалидна
            m_player->gainEnergy(card->getCurrentCost());
            return false;
        }
    }

    // ── Применяем эффекты карты ───────────────────────────
    resolveCardEffects(*card, targetIndex);

    // ── Перемещаем карту из руки в сброс ──────────────────
    m_player->getDeck().playCard(static_cast<size_t>(cardIndex));

    // Публикуем событие
    m_eventBus.publish({EventType::CardPlayed, {
        {"card_id", card->getId()},
        {"card_name", card->getName()},
        {"target", targetIndex}
    }});

    // Проверяем, не убили ли мы всех врагов
    checkCombatEnd();

    return true;
}

// ══════════════════════════════════════════════════════════
// РАЗРЕШЕНИЕ ЭФФЕКТОВ КАРТЫ
// ══════════════════════════════════════════════════════════

void CombatManager::resolveCardEffects(const Card& card, int targetIndex) {
    for (const auto& effect : card.getEffects()) {
        switch (effect.type) {

            // ── УРОН ──────────────────────────────────────
            case CardEffectType::Damage: {
                if (card.getTarget() == CardTarget::SingleEnemy) {
                    Enemy* target = getAliveEnemy(targetIndex);
                    if (target) {
                        int actualDamage = target->takeDamage(effect.value);
                        m_eventBus.publish({EventType::DamageDealt, {
                            {"amount", actualDamage},
                            {"target", targetIndex}
                        }});
                    }
                }
                else if (card.getTarget() == CardTarget::AllEnemies) {
                    for (int i = 0; i < static_cast<int>(m_enemies.size()); ++i) {
                        if (m_enemies[i]->isAlive()) {
                            int actualDamage = m_enemies[i]->takeDamage(effect.value);
                            m_eventBus.publish({EventType::DamageDealt, {
                                {"amount", actualDamage},
                                {"target", i}
                            }});
                        }
                    }
                }
                break;
            }

            // ── ЧИСТЫЙ УРОН (игнорирует файрвол) ─────────
            case CardEffectType::DamageIgnoreBlock: {
                if (card.getTarget() == CardTarget::SingleEnemy) {
                    Enemy* target = getAliveEnemy(targetIndex);
                    if (target) {
                        int actualDamage = target->takeDamage(effect.value, true);
                        m_eventBus.publish({EventType::DamageDealt, {
                            {"amount", actualDamage},
                            {"target", targetIndex},
                            {"piercing", true}
                        }});
                    }
                }
                break;
            }

            // ── ФАЙРВОЛ (БРОНЯ) ───────────────────────────
            case CardEffectType::Block: {
                m_player->gainBlock(effect.value);
                m_eventBus.publish({EventType::BlockGained, {
                    {"amount", effect.value}
                }});
                break;
            }

            // ── ЭНЕРГИЯ ───────────────────────────────────
            case CardEffectType::GainEnergy: {
                m_player->gainEnergy(effect.value);
                m_eventBus.publish({EventType::EnergyChanged, {
                    {"energy", m_player->getEnergy()}
                }});
                break;
            }

            // ── ДОБОР КАРТ ────────────────────────────────
            case CardEffectType::DrawCards: {
                m_player->getDeck().drawCards(effect.value, m_rng);
                m_eventBus.publish({EventType::CardDrawn, {
                    {"count", effect.value}
                }});
                break;
            }

            // ── СТАТУС НА ВРАГА ───────────────────────────
            case CardEffectType::ApplyStatus: {
                if (card.getTarget() == CardTarget::SingleEnemy) {
                    Enemy* target = getAliveEnemy(targetIndex);
                    if (target) {
                        target->applyStatus(effect.statusId, effect.value);
                        m_eventBus.publish({EventType::StatusApplied, {
                            {"status", effect.statusId},
                            {"stacks", effect.value},
                            {"target", targetIndex}
                        }});
                    }
                }
                else if (card.getTarget() == CardTarget::AllEnemies) {
                    for (int i = 0; i < static_cast<int>(m_enemies.size()); ++i) {
                        if (m_enemies[i]->isAlive()) {
                            m_enemies[i]->applyStatus(effect.statusId, effect.value);
                        }
                    }
                }
                break;
            }

            // ── УРОН СЕБЕ (для Разгона процессора) ────────
            case CardEffectType::SelfDamage: {
                int actualDamage = m_player->takeDamage(effect.value, true);
                m_eventBus.publish({EventType::DamageReceived, {
                    {"amount", actualDamage},
                    {"source", std::string("self")}
                }});
                break;
            }
        }
    }
}

// ══════════════════════════════════════════════════════════
// КОНЕЦ ХОДА ИГРОКА
// ══════════════════════════════════════════════════════════

void CombatManager::endPlayerTurn() {
    if (m_phase != TurnPhase::PlayerAction) return;

    m_phase = TurnPhase::PlayerTurnEnd;

    // 1. Сбросить всю несыгранную руку в сброс
    m_player->endTurn();

    // 2. Тик статусов на игроке (если есть дебаффы)
    m_player->tickStatuses(TurnPhase::PlayerTurnEnd);

    // 3. Фаза врагов
    executeEnemyTurns();

    // 4. Тик статусов на всех врагах (Утечка и т.д.)
    tickAllStatuses();

    // 5. Проверка конца боя
    checkCombatEnd();

    // 6. Если бой не окончен — начинаем новый ход игрока
    if (!isCombatOver()) {
        startPlayerTurn();
    }
}

// ══════════════════════════════════════════════════════════
// ХОД ВРАГОВ
// ══════════════════════════════════════════════════════════

void CombatManager::executeEnemyTurns() {
    m_phase = TurnPhase::EnemyAction;

    for (auto& enemy : m_enemies) {
        if (!enemy->isAlive()) continue;
        if (!m_player->isAlive()) break; // Игрок мёртв — стоп

        // Сбрасываем файрвол врага перед его ходом
        enemy->resetBlock();

        // Враг выполняет своё намерение
        enemy->executeIntent(*m_player);

        m_eventBus.publish({EventType::DamageReceived, {
            {"source", enemy->getName()}
        }});
    }
}

// ══════════════════════════════════════════════════════════
// ТИК СТАТУСОВ
// ══════════════════════════════════════════════════════════

void CombatManager::tickAllStatuses() {
    for (auto& enemy : m_enemies) {
        if (!enemy->isAlive()) continue;

        int leakDamage = enemy->tickStatuses(TurnPhase::EnemyTurnEnd);
        if (leakDamage > 0) {
            m_eventBus.publish({EventType::DamageDealt, {
                {"amount", leakDamage},
                {"source", std::string("leak")}
            }});
        }
    }
}

// ══════════════════════════════════════════════════════════
// ПРОВЕРКА КОНЦА БОЯ
// ══════════════════════════════════════════════════════════

void CombatManager::checkCombatEnd() {
    // Проверяем смерть игрока
    if (!m_player->isAlive()) {
        m_phase = TurnPhase::CombatEnd;
        m_playerWon = false;
        m_eventBus.publish({EventType::PlayerDied, {}});
        m_eventBus.publish({EventType::CombatEnded, {
            {"player_won", false}
        }});
        return;
    }

    // Проверяем, все ли враги мертвы
    if (countAliveEnemies() == 0) {
        m_phase = TurnPhase::CombatEnd;
        m_playerWon = true;
        m_eventBus.publish({EventType::CombatEnded, {
            {"player_won", true},
            {"turns", m_turnNumber}
        }});
    }
}

// ══════════════════════════════════════════════════════════
// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
// ══════════════════════════════════════════════════════════

Enemy* CombatManager::getAliveEnemy(int index) {
    if (index < 0 || index >= static_cast<int>(m_enemies.size())) return nullptr;
    if (!m_enemies[index]->isAlive()) return nullptr;
    return m_enemies[index].get();
}

int CombatManager::countAliveEnemies() const {
    int count = 0;
    for (const auto& enemy : m_enemies) {
        if (enemy->isAlive()) count++;
    }
    return count;
}

std::vector<const Enemy*> CombatManager::getAliveEnemies() const {
    std::vector<const Enemy*> result;
    for (const auto& enemy : m_enemies) {
        if (enemy->isAlive()) {
            result.push_back(enemy.get());
        }
    }
    return result;
}

} // namespace engine
