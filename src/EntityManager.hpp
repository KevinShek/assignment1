#pragma once

#include "Entity.hpp"

using EntityVec = std::vector<std::shared_ptr<Entity>>;

class EntityManager
{
    EntityVec m_entities;
    EntityVec m_entitiesToAdd;
    std::map<std::string, EntityVec> m_entityMap;
    size_t m_totalEntities = 0;

    void removeDeadEntities(EntityVec &vec)
    {
        // remove all dead entities from the input vector
        // this is called by the update() function
        // std::remove_if
        EntityVec newVec;
        for (auto &entity : vec)
        {
            // vec.erase(std::remove_if(vec.begin(), vec.end(), [](Entity& ent) { return ent.m_active == false; }), vec.end());
            if (entity.get()->m_active == true)
            {
                newVec.push_back(entity);
            }
        }
        vec = newVec;
    }

public:
    EntityManager() = default;

    void update()
    {
        // add entities from m_entitiesToAdd the proper location(s)
        // - add them to the vector of all entities
        // - add them to the vector inside the map, with the tag as a key
        for (auto& entity : m_entitiesToAdd)
        {
            m_entities.push_back(entity);
        }

        // remove dead entities from the vector of all entities
        removeDeadEntities(m_entities);

        // remove dead entities from each vector in the entity map
        // C++20 way of iterating through [key,value] pairs in a map
        for (auto &[tag, entityVec] : m_entityMap)
        {
            removeDeadEntities(entityVec);
        }
        m_entitiesToAdd.clear();
    }

    std::shared_ptr<Entity> addEntity(const std::string &tag)
    {
        // create the entity shared pointer
        auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

        // add it to the vec of all entities
        m_entitiesToAdd.push_back(entity);

        // add it to the entity map
        if (m_entityMap.find(tag) == m_entityMap.end())
        {
            m_entityMap[tag] = EntityVec();
        }
        m_entityMap[tag].push_back(entity);

        return entity;
    }

    // grab all the entities
    const EntityVec &getEntities()
    {
        return m_entities;
    }

    // bookkeep all the entities within the manager
    const EntityVec &getEntities(const std::string &tag)
    {
        if (m_entityMap.find(tag) == m_entityMap.end())
        {
            m_entityMap[tag] = EntityVec();
        }
        return m_entityMap[tag];
    }

    const std::map<std::string, EntityVec> &getEntityMap()
    {
        return m_entityMap;
    }
};