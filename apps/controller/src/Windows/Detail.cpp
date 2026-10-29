#include "Detail.h"
#include "DetailFader.h"
#include "DetailList.h"
#include "DetailEnvelope.h"
#include "DetailVerticalFader4.h"
#include "DetailXyPad.h"

Detail::Detail(const Control &newControl, MainDelegate &newDelegate)
    : control(newControl),
      delegate(newDelegate),
      component(nullptr),
      lock(nullptr),
      locked(false)
{
    setName("detail");

    component =
        ControlComponent::createDetailControlComponent(control, delegate);

    if (component) {
        addAndMakeVisible(component);
    }

    lock = new Lock(locked);

    if (lock) {
        addAndMakeVisible(lock);

        lock->onClick = [this] {
            if (!locked) {
                delegate.lockDetail();
            } else {
                delegate.closeDetail();
            }
        };
    }

    dimmedSection = new VoidSpace();

    if (dimmedSection) {
        addAndMakeVisible(dimmedSection);

        dimmedSection->onClick = [this] {
            delegate.closeDetail();
        };
    }
}

Detail::~Detail()
{
    tearDownKnobs();
}

/*
 * A dial on one of the control's values. The Control and its values live in
 * the list because the component holds references to them; the knob is
 * registered on the parameter map like any control, so it follows its value
 * wherever it is changed from, and is taken off again when the window closes.
 */
ControlComponent *
    Detail::setUpKnob(const char *name, uint8_t handle, uint8_t potId)
{
    /* A preset declares as many values as its JSON carried, which can be fewer
     * than the control type implies. There is no knob for a value that is not
     * there. */
    if (handle >= control.values.size()) {
        return (nullptr);
    }

    knobs.emplace_back();

    Knob &knob = knobs.back();

    knob.control.setId(control.getId() + 500 + knobs.size());
    knob.control.setType(Control::Type::Knob);
    knob.control.setName(name);
    knob.control.setColour(control.getColour565());

        knob.values.push_back(control.getValue(handle));
    knob.values[0].setControl(&knob.control);
    /* A copied value still names the original as its message's owner, and
     * that is what the parameter map links when a script re-targets the
     * message. It has to be this copy, or the link lands on the value the
     * knob is standing in for. */
    knob.values[0].message.setControlValue(&knob.values[0]);
    knob.values[0].setNextDestination(nullptr);
    knob.control.setValues(knob.values);

    knob.component =
        ControlComponent::createControlComponent(knob.control, delegate);

    if (!knob.component) {
        knobs.pop_back();
        return (nullptr);
    }

    knob.control.setComponent(knob.component);
    knob.control.addToParameterMap(knob.control.values[0]);
    knob.component->updateValueFromParameterMap();
    knob.component->assignPot(potId, knob.control.values[0].getPotSteps());
    addAndMakeVisible(knob.component);

    return (knob.component);
}

void Detail::resizeKnob(ControlComponent *knob,
                        uint16_t x,
                        uint16_t y,
                        uint16_t width,
                        uint16_t height)
{
    if (knob) {
        knob->setBounds(x, y, width, height);
    }
}

void Detail::tearDownKnobs(void)
{
    for (Knob &knob : knobs) {
        if (knob.component) {
            knob.control.removeFromParameterMap(knob.control.values[0]);
            knob.component->releasePot();
        }
    }
}

void Detail::setLocked(void)
{
    locked = true;
    lock->setLocked(true);
}

bool Detail::isLocked(void)
{
    return (locked);
}

Detail *Detail::createDetail(const Control &control, MainDelegate &newDelegate)
{
    Detail *detail = nullptr;

    switch (control.getType()) {
        case Control::Type::Fader:
            detail = new DetailFader(control, newDelegate);
            break;

        case Control::Type::List:
            detail = new DetailList(control, newDelegate);
            break;

        case Control::Type::Vfader:
            detail = new DetailVerticalFader4(control, newDelegate);
            break;

        case Control::Type::XyPad:
            detail = new DetailXyPad(control, newDelegate);
            break;

        /* Every envelope opens the same window; what differs is how many
         * dials it carries, which DetailEnvelope reads off the control. */
        case Control::Type::Adsr:
        case Control::Type::Ahdsr:
        case Control::Type::Adssr:
        case Control::Type::Adr:
        case Control::Type::Ar:
        case Control::Type::Dx7envelope:
            detail = new DetailEnvelope(control, newDelegate);
            break;

        default:
            break;
    }

    return (detail);
}
