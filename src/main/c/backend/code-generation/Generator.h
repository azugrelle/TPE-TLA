#ifndef GENERATOR_HEADER
#define GENERATOR_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include "../domain-specific/Calculator.h"
#include <stdio.h>

/** Initialize module's internal state. */
ModuleDestructor initializeGeneratorModule(void);

/**
 * Generates a self-contained HTML document (with inline SVG) to standard
 * output, drawing one analog clock per declared ClockState. The document has
 * no external dependencies and can be opened directly in any browser.
 */
void generateHTML(ClockStateList * clocks);

#endif
