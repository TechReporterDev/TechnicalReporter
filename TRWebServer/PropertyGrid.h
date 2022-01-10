#include "stdafx.h"
#include "Property.h"

template<class... PropertyFactories>
class PropertyGrid
{
public:
    using Column = boost::variant<PropertyArray<PropertyFactories>...>;
    PropertyGrid(std::vector<Column> columns);

    size_t  getColumnCount() const;
    size_t  getItemCount(size_t column) const;
    void    addColumn(Column column);
    void    removeColumn(size_t position);
    Column  getColumn(size_t position) const;

private:
    std::vector<Column> m_columns;
};

template<class Column>
size_t getItemCount(const Column& column);

///////////////
// implementation
///////////////

template<class... PropertyFactories>
PropertyGrid<PropertyFactories...>::PropertyGrid(std::vector<Column> columns):
    m_columns(std::move(columns))
{
}

template<class... PropertyFactories>
size_t PropertyGrid<PropertyFactories...>::getColumnCount() const
{
    return m_columns.size();
}

struct GetItemCount: boost::static_visitor<size_t>
{
    template<class PropertyArrayType>
    size_t operator()(const PropertyArrayType& propertyArray) const
    {
        return propertyArray.size();
    }
};

template<class... PropertyFactories>
size_t PropertyGrid<PropertyFactories...>::getItemCount(size_t column) const
{
    _ASSERT(column < m_columns.size());
    return boost::apply_visitor(GetItemCount(), m_columns[column]);
}

template<class... PropertyFactories>
void PropertyGrid<PropertyFactories...>::addColumn(Column column)
{
    return m_columns.push_back(std::move(column));
}

template<class... PropertyFactories>
void PropertyGrid<PropertyFactories...>::removeColumn(size_t position)
{
    return m_columns.erase(m_columns.begin() + position);
}

template<class... PropertyFactories>
typename PropertyGrid<PropertyFactories...>::Column PropertyGrid<PropertyFactories...>::getColumn(size_t position) const
{
    return m_columns[position];
}