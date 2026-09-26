#pragma once

#include "Component.h"
#include "Lock.h"
#include "VoidSpace.h"
#include "Model/Control.h"
#include "ControlComponent.h"
#include "System.h"
#include "MainDelegate.h"
#include <list>
#include <vector>

class DetailFader;
class DetailList;
class DetailEnvelope;

class Detail : public Component
{
public:
    Detail(const Control &newControl, MainDelegate &newDelegate);
    virtual ~Detail();

    void setLocked(void);
    bool isLocked(void);

    virtual void paint(Graphics &g) = 0;
    virtual void resized(void) = 0;

    static Detail *createDetail(const Control &control,
                                MainDelegate &newDelegate);

protected:
    /**
     * @brief A dial on one of the control's values, on a knob of its own.
     *
     * Every detail that carries knobs used to write this out per knob - a
     * Control, a vector of values, and fourteen lines of wiring, copied once
     * per value and four times per window. The knob's Control and its values
     * are owned here, because the component holds a reference to them for as
     * long as the window is open.
     *
     * @param name what the dial is labelled
     * @param handle which of the control's values it turns
     * @param potId the knob it answers to
     * @return the component, or nullptr where the preset declared no such
     *         value
     */
    ControlComponent *
        setUpKnob(const char *name, uint8_t handle, uint8_t potId);

    /** Moves a knob setUpKnob() made, or does nothing when there is none. */
    void resizeKnob(ControlComponent *knob,
                    uint16_t x,
                    uint16_t y,
                    uint16_t width,
                    uint16_t height);

    /** Unregisters and releases every knob. Called from the destructor. */
    void tearDownKnobs(void);

    const Control &control;
    MainDelegate &delegate;
    Component *component;
    Lock *lock;
    VoidSpace *dimmedSection;
    bool locked;

private:
    struct Knob {
        Control control;
        std::vector<ControlValue> values;
        ControlComponent *component = nullptr;
    };

    /* A list, not a vector: a knob's component holds a reference to its
     * Control, so the Controls must not move when another knob is added. */
    std::list<Knob> knobs;
};
