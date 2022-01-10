#include "stdafx.h"
#include "ComplianceContentView.h"
#include "Dialog.h"
#include "Application.h"
namespace {

std::unique_ptr<Wt::WImage> get_rule_image(const TR::XML::XmlRuleNode& ruleNode)
{
    const char* image_path = nullptr;
    switch (ruleNode.get_check_result())
    {
    case TR::XML::XmlRuleNode::CheckResult::CHECKED:
        image_path = "icons/sun01.png";
        break;

    case TR::XML::XmlRuleNode::CheckResult::FAILED:
        image_path = "icons/snow.png";
        break;

    case TR::XML::XmlRuleNode::CheckResult::EXCEPTED:
        image_path = "icons/w_cloud.png";
        break;

    default:
        _ASSERT(false);
        throw std::logic_error("Invalid rule checking result");
    }

    return std::make_unique<Wt::WImage>(Wt::WLink(image_path));
}

std::unique_ptr<Wt::WImage> get_check_image(const TR::XML::XmlRuleCheck& ruleCheck)
{
    const char* image_path = "icons/sun01.png";

    if (!ruleCheck.is_checked())
    {
        image_path = "icons/snow.png";
    }

    if(ruleCheck.is_excepted())
    {
        image_path = "icons/w_cloud.png";
    }

    return std::make_unique<Wt::WImage>(Wt::WLink(image_path));
}

class RuleDetailsDlg: public Dialog
{
public:
    RuleDetailsDlg(const TR::XML::XmlRuleNode& ruleNode)
    {
        setWindowTitle("Rule Checking Details");
        setButtons(BTN_CLOSE);
        setHeight(400);
        setWidth(350);

        contents()->setOverflow(Wt::Overflow::Auto);
        for (auto& ruleCheck : ruleNode.get_rule_checks())
        {
            auto checkWidget = contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
            
            auto ruleImage = get_check_image(ruleCheck);            
            ruleImage->resize(20, 20);
            checkWidget->addWidget(std::move(ruleImage));

            auto idText = checkWidget->addWidget(std::make_unique<Wt::WText>(ruleCheck.get_id()));
            idText->setMargin(5, Wt::Side::Left);
        }
    }
};

class RulePanel: public Wt::WContainerWidget
{
public:
    RulePanel(const TR::XML::XmlRuleNode& ruleNode):
        m_ruleNode(ruleNode)
    {
        setStyleClass("tr-rule-panel tr-rule-panel-default");
        
        auto image = get_rule_image(m_ruleNode);
        image->setStyleClass("tr-rule-panel-image");
        addWidget(std::move(image));

        auto detailsBtn = addWidget(std::make_unique<Wt::WPushButton>("Details"));
        detailsBtn->setStyleClass("tr-rule-panel-button");
        detailsBtn->clicked().connect([this](Wt::WMouseEvent){
            auto dlg = new RuleDetailsDlg(m_ruleNode);
            dlg->show();
        });

        auto caption = addWidget(std::make_unique<Wt::WContainerWidget>());
        caption->setStyleClass("tr-rule-panel-caption");
        caption->addWidget(std::make_unique<Wt::WText>(ruleNode.get_bound_def().get_caption()));

        auto description = addWidget(std::make_unique<Wt::WContainerWidget>());
        description->setStyleClass("tr-rule-panel-description");
        description->addWidget(std::make_unique<Wt::WText>(ruleNode.get_bound_def().get_description()));

        auto id = boost::lexical_cast<std::string>(&ruleNode);
        setId(id);

        implementJavaScript
            (&RulePanel::onMouseOver,
            "{"
            "var s = $('#" + id + "');"
            "s.attr('class', 'tr-rule-panel tr-rule-panel-selected');"
            "}");

        implementJavaScript
            (&RulePanel::onMouseOut,
            "{"
            "var s = $('#" + id + "');"
            "s.attr('class', 'tr-rule-panel tr-rule-panel-default');"
            "}");

        mouseWentOver().connect(this, &RulePanel::onMouseOver);
        mouseWentOut().connect(this, &RulePanel::onMouseOut);
    }

private:
    // used to prevent optimization of identical function in Release build
    static int optimization_prevention;

    void onMouseOver()
    {
        //used to install JavaScript
        optimization_prevention += 1;
    }

    void onMouseOut()
    {
        //used to install JavaScript
        optimization_prevention += 2;       
    }

    const TR::XML::XmlRuleNode& m_ruleNode;
};

int RulePanel::optimization_prevention = 0;

} //namespace {

ComplianceContentView::ComplianceContentView(std::shared_ptr<const TR::ComplianceContent> content):
    m_content(content)
{
    setPadding(5);
    setOverflow(Wt::Overflow::Auto);
    reset();
}

void ComplianceContentView::update(std::shared_ptr<const TR::Content> content)
{
    m_content = std::dynamic_pointer_cast<const TR::ComplianceContent>(content);
    reset();
}

void ComplianceContentView::reset()
{
    clear();
    auto& compliance_doc = m_content->get_compliance_doc();
    for (auto& rule_node : compliance_doc.get_rules())
    {
        auto rulePanel = addWidget(std::make_unique<RulePanel>(rule_node));     
    }
}