#include "MainComponent.h"
#include "helpers.h"

MainComponent::MainComponent()
    : usbDevOutput(MidiInterface::Type::MidiUsbDev, 0)
{
    // Set the component name
    setName("mainComponent");

    // Add an example of graphics object

    // Pads
    for (uint8_t i = 0; i < numPads; i++) {
        pad[i] = new Pad;
        pad[i]->setColour(((i % 4) == 0) ? Colours565::yellow
                                         : Colours565::red);
        pad[i]->setState(((i % 3) == 0) ? true : false);
        addAndMakeVisible(pad[i]);
    }

    // Custom component
    drawings = new Drawings;
    addAndMakeVisible(drawings);

    // Knobs
    for (uint8_t i = 0; i < numKnobs; i++) {
        knob[i] = new Knob();
        knob[i]->setColour(Colours565::darkslateblue);
        knob[i]->setRange(-64.0f, 63.0f);
        knob[i]->setValue(0.0f);
        knob[i]->assignPot(i);
        addAndMakeVisible(knob[i]);
    }

    // Set up a slider (an example of a component)
    for (uint8_t i = 0; i < numBars; i++) {
        bar[i] = new BarVertical;
        bar[i]->setColour(Colours565::peru);
        bar[i]->setRange(-64.0f, 63.0f);
        bar[i]->setValue(0.0f);
        bar[i]->assignPot(i);
        bar[i]->assignPot(i + numKnobs);
        addAndMakeVisible(bar[i]);
    }

    assignAllPots();

    setBounds(0, 0, 1024, 560);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(Graphics &g)
{
    g.fillAll(Colours565::black);
    return;
    g.user().printText(0,
                       40,
                       "Demo Application",
                       getWidth(),
                       TextAlign::center,
                       Colours565::white,
                       TextSize::large);
}

void MainComponent::resized()
{
    for (uint8_t i = 0; i < numPads; i++) {
        pad[i]->setBounds(48 + (122 * i), 40, 70, 70);
    }

    drawings->setBounds(122, 150, 780, 140);

    for (uint8_t i = 0; i < numKnobs; i++) {
        knob[i]->setBounds(35 + (170 * i), 300, 100, 100);
    }

    for (uint8_t i = 0; i < numBars; i++) {
        bar[i]->setBounds(50 + (170 * i), 430, 70, 120);
    }
}

void MainComponent::onPotChange(const PotEvent &potEvent)
{
    System::logger.write(LOG_ERROR,
                         "potChange: potId=%d, relativeChange=%d",
                         potEvent.getPotId(),
                         potEvent.getRelativeChange());
}

void MainComponent::onPotTouchDown(const PotEvent &potEvent)
{
    System::logger.write(
        LOG_ERROR, "potTouchDown: potId=%d", potEvent.getPotId());
}

void MainComponent::onPotTouchUp(const PotEvent &potEvent)
{
    System::logger.write(
        LOG_ERROR, "potTouchUp: potId=%d", potEvent.getPotId());
}
