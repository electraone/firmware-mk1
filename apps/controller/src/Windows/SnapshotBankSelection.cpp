#include "SnapshotBankSelection.h"
#include "Window.h"

SnapshotBankSelection::SnapshotBankSelection(
    SnapshotsWindowDelegate &newDelegate,
    uint8_t newBankNumber,
    uint32_t newColour,
    uint32_t newActiveColour)
    : Selection(newBankNumber, newColour, newActiveColour),
      delegate(newDelegate)
{
    setName("SnapshotBankSelection");

    for (uint8_t i = 0; i < 12; i++) {
        if (button[i]) {
            char label[10];
            snprintf(label, sizeof(label), "Bank %d", i + 1);
            button[i]->setLabel(label);
            button[i]->onClick = [this, i]() {
                setActiveBank(i);
                return (true);
            };
        }
    }
}

void SnapshotBankSelection::setActiveBank(uint8_t newBankNumber)
{
    uint8_t previousBankNumber = active;
    active = newBankNumber;
    delegate.switchSnapshotBank(active);
    button[previousBankNumber]->setSelected(false);
    button[active]->setSelected(true);
    getWindow()->repaint();
}
