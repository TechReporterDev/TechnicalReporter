#include "MainFrame.h"
#include "PropertyDialog.h"
#include "PropertySheet.h"
#include "CommonProperty.h"

class PropertyBox: public Dialog
{
public:
    PropertyBox(stl_tools::flag_type<DialogButton> buttons, std::wstring title, std::wstring message, Wt::Icon icon = Wt::Icon::None);

    void addString(std::wstring name, std::wstring description, std::wstring value, std::wstring prompt = L"");
    void addSwitch(std::wstring name, std::wstring description, std::vector<std::wstring> cases, int value = 0);
    void addCheck(std::wstring name, std::wstring description, bool value = false);
    void addText(std::wstring name, std::wstring description, std::wstring value, bool readOnly = false);

    void show(std::function<void()> accept_handler = nullptr, std::function<void()> reject_handler = nullptr);

    template<class T>
    T getValue(int position) const
    {
        auto value = std::dynamic_pointer_cast<SingleValue<T>>(m_commonProperty.shareValue()->getMember(position));
        if (!value)
        {
            throw std::logic_error("Invalid value type");
        }
        return value->get();
    }   

protected:
    // Dialog override
    virtual void apply();
    virtual void cancel();

    CommonProperty m_commonProperty;
    std::function<void()> m_accept_handler;
    std::function<void()> m_reject_handler;
};