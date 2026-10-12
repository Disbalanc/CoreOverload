#pragma once

#include "engine/entities/Artifact.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace engine {

class ArtifactDatabase {
public:
    static ArtifactDatabase& instance();

    bool loadFromFile(const std::string& filePath);

    const ArtifactTemplate* getTemplate(const std::string& id) const;
    std::vector<std::string> getAllIds() const;
    std::vector<std::string> getIdsByRarity(CardRarity rarity) const;

    std::unique_ptr<Artifact> createArtifact(const std::string& id) const;

private:
    ArtifactDatabase() = default;
    std::unordered_map<std::string, ArtifactTemplate> m_templates;
};

} // namespace engine
