#pragma once

namespace Wt
{
    template <class Button, class... Args>
    Button* addButton(Wt::WToolBar& toolbar, Args&&... args)
    {
        auto button = std::make_unique<Button>(std::forward<Args>(args)...);
        auto pointer = button.get();
        toolbar.addButton(std::move(button));
        return pointer;
    }

    template<class Layout = Wt::WVBoxLayout>
    inline Layout* clear(Wt::WContainerWidget* container)
    {
        auto layout = std::make_unique<Layout>();        
        if (auto prevLayout = dynamic_cast<Wt::WBoxLayout*>(container->layout()))
        {
            int left = 0, right = 0, top = 0, bottom = 0;            
            prevLayout->getContentsMargins(&left, &right, &top, &bottom);
            layout->setContentsMargins(left, right, top, bottom);
            layout->setSpacing(prevLayout->spacing());
        }        

        container->clear();
        return container->setLayout(std::move(layout));
    }
}