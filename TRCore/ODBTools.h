#pragma once
#include <odb\database.hxx>

namespace odb
{
    template <typename T>
    inline typename object_traits<T>::id_type persist(database& db, T& obj)
    {
        return db.persist(obj);
    }
}