#include "PluginEditor.h"
#include "BinaryData.h"

static juce::RangedAudioParameter& getParam (juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    auto* p = apvts.getParameter (id);
    jassert (p != nullptr);
    return *p;
}

DoctorsDistortionEditor::DoctorsDistortionEditor(DoctorsDistortionProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      aF1Cutoff(getParam(p.apvts, "filter1Cutoff"), rF1Cutoff, nullptr),
      aF2Cutoff(getParam(p.apvts, "filter2Cutoff"), rF2Cutoff, nullptr),
      aF1Reso(getParam(p.apvts, "filter1Reso"), rF1Reso, nullptr),
      aF2Reso(getParam(p.apvts, "filter2Reso"), rF2Reso, nullptr),
      aDistDrive(getParam(p.apvts, "distDrive"), rDistDrive, nullptr),
      aDistMix(getParam(p.apvts, "distMix"), rDistMix, nullptr),
      aDistTone(getParam(p.apvts, "distTone"), rDistTone, nullptr),
      aLimThresh(getParam(p.apvts, "limiterThresh"), rLimThresh, nullptr),
      aLimRelease(getParam(p.apvts, "limiterRelease"), rLimRelease, nullptr),
      aDryWet(getParam(p.apvts, "dryWet"), rDryWet, nullptr),
      aInGain(getParam(p.apvts, "inputGain"), rInGain, nullptr),
      aOutGain(getParam(p.apvts, "outputGain"), rOutGain, nullptr),
      aF1Type(getParam(p.apvts, "filter1Type"), rF1Type, nullptr),
      aF2Type(getParam(p.apvts, "filter2Type"), rF2Type, nullptr),
      aRouting(getParam(p.apvts, "routingMode"), rRouting, nullptr),
      aDistType(getParam(p.apvts, "distType"), rDistType, nullptr),
      aLimType(getParam(p.apvts, "limiterType"), rLimType, nullptr),
      webComponent(juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
                                  .withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory)))
          .withOptionsFrom (rF1Cutoff).withOptionsFrom (rF2Cutoff)
          .withOptionsFrom (rF1Reso).withOptionsFrom (rF2Reso)
          .withOptionsFrom (rDistDrive).withOptionsFrom (rDistMix).withOptionsFrom (rDistTone)
          .withOptionsFrom (rLimThresh).withOptionsFrom (rLimRelease)
          .withOptionsFrom (rDryWet).withOptionsFrom (rInGain).withOptionsFrom (rOutGain)
          .withOptionsFrom (rF1Type).withOptionsFrom (rF2Type)
          .withOptionsFrom (rRouting).withOptionsFrom (rDistType).withOptionsFrom (rLimType)
          .withNativeFunction ("getMeters", [this](const auto&, auto complete) {
              auto* obj = new juce::DynamicObject();
              obj->setProperty("inL", processorRef.inputLevelL.load());
              obj->setProperty("inR", processorRef.inputLevelR.load());
              obj->setProperty("outL", processorRef.outputLevelL.load());
              obj->setProperty("outR", processorRef.outputLevelR.load());
              complete (juce::var (obj));
          })
          .withResourceProvider([this] (const juce::String& url) -> std::optional<juce::WebBrowserComponent::Resource>
          {
              // Strip any leading slash (browser sends /styles.css, we want styles.css)
              auto path = url.trimCharactersAtStart ("/");
              if (path.isEmpty()) path = "index.html";
              int dataSize = 0;
              const char* data = nullptr;
              
              if (path == "index.html") { data = BinaryData::index_html; dataSize = BinaryData::index_htmlSize; }
              else if (path == "styles.css") { data = BinaryData::styles_css; dataSize = BinaryData::styles_cssSize; }
              else if (path == "script.js") { data = BinaryData::script_js; dataSize = BinaryData::script_jsSize; }
              
              if (data != nullptr)
              {
                  juce::String mime = "text/plain";
                  if (path.endsWithIgnoreCase(".html")) mime = "text/html";
                  else if (path.endsWithIgnoreCase(".css")) mime = "text/css";
                  else if (path.endsWithIgnoreCase(".js")) mime = "text/javascript";
                  
                  std::vector<std::byte> vec((const std::byte*)data, (const std::byte*)data + dataSize);
                  return juce::WebBrowserComponent::Resource { std::move(vec), mime };
              }
              return std::nullopt;
          }, juce::String {"http://localhost"})
      )
{
    // Make sure size perfectly matches the 900x540 web canvas plus body padding 50px
    setSize(960, 480);
    setResizable(true, true);
    setResizeLimits(820, 400, 1600, 1100);

    addAndMakeVisible(webComponent);
    webComponent.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
}

DoctorsDistortionEditor::~DoctorsDistortionEditor()
{
}

void DoctorsDistortionEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0d0d0d)); // Matches web body background color
}

void DoctorsDistortionEditor::resized()
{
    webComponent.setBounds(getLocalBounds());
}

juce::AudioProcessorEditor* DoctorsDistortionProcessor::createEditor()
{
    return new DoctorsDistortionEditor(*this);
}
