#include "stdafx.h"
#include "Layout.h"

Wt::WVBoxLayout* centerLayout(Wt::WContainerWidget* parent)
{
    _ASSERT(parent);

    auto vbox = parent->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->addStretch(1);
    auto hbox = vbox->addLayout(std::make_unique<Wt::WHBoxLayout>());
    vbox->addStretch(1);

    hbox->addStretch(1);
    auto center = hbox->addLayout(std::make_unique<Wt::WVBoxLayout>());
    hbox->addStretch(1);

    return center;
}