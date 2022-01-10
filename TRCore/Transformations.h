#pragma once
#include "Selections.h"
#include "Groupings.h"
#include "Convertions.h"
#include "Trackings.h"
namespace TR { namespace Core {

struct Transformations
{
    Selections      m_selections;
    Groupings       m_groupings;
    Convertions     m_convertions;
    Trackings       m_trackings;
};

}} //namespace TR { namespace Core {