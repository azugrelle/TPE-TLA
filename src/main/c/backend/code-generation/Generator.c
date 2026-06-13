#include "Generator.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void _shutdownGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Generator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeGeneratorModule(void) {
	_logger = createLogger("Generator");
	return _shutdownGeneratorModule;
}

/* PRIVATE FUNCTIONS */

#define HOUR_MARKS 12

static const char * ROMAN_NUMERALS[HOUR_MARKS] = {
	"I", "II", "III", "IV", "V", "VI",
	"VII", "VIII", "IX", "X", "XI", "XII"
};

static double toRadians(double degrees) {
	return degrees * M_PI / 180.0;
}

/** Maps a Color enum to its CSS/SVG hexadecimal representation. */
static const char * colorToHex(Color color) {
	switch (color) {
		case COLOR_BLACK: return "#000000";
		case COLOR_WHITE: return "#FFFFFF";
		case COLOR_GREEN: return "#2ECC71";
		case COLOR_RED:   return "#E74C3C";
		case COLOR_BLUE:  return "#3498DB";
	}
	return "#000000";
}

/** Emits the 12 clock-face numbers, Arabic or Roman according to the style. */
static void generateNumbers(ClockState * state, FILE * output) {
	const char * fill = colorToHex(state->style.borderColor);
	for (int i = 0; i < HOUR_MARKS; ++i) {
		double angle = toRadians(i * 30.0);
		double x = sin(angle) * 72.0;
		double y = -cos(angle) * 72.0 + 5.0;
		int number = (i == 0) ? 12 : i;
		char label[8];
		if (state->style.numbers == NUMBER_ROMAN) {
			snprintf(label, sizeof(label), "%s", ROMAN_NUMERALS[number - 1]);
		} else {
			snprintf(label, sizeof(label), "%d", number);
		}
		fprintf(output,
			"        <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\" font-size=\"14\" fill=\"%s\" font-family=\"sans-serif\">%s</text>\n",
			x, y, fill, label);
	}
}

/** Emits the inline SVG drawing for a single clock. */
static void generateClockSVG(ClockState * state, FILE * output) {
	const char * bgColor = colorToHex(state->style.bgColor);
	const char * borderColor = colorToHex(state->style.borderColor);
	const char * handColor = colorToHex(state->style.handColor);

	fprintf(output, "      <svg viewBox=\"-105 -105 210 210\" width=\"200\" height=\"200\">\n");

	// Background and border.
	fprintf(output, "        <circle r=\"100\" fill=\"%s\" stroke=\"%s\" stroke-width=\"4\"/>\n", bgColor, borderColor);

	// 12 hour marks (short radial lines near the rim).
	for (int i = 0; i < HOUR_MARKS; ++i) {
		double angle = toRadians(i * 30.0);
		double s = sin(angle);
		double c = cos(angle);
		fprintf(output,
			"        <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"%s\" stroke-width=\"2\"/>\n",
			s * 85.0, -c * 85.0, s * 95.0, -c * 95.0, borderColor);
	}

	// Numbers.
	generateNumbers(state, output);

	// Hour hand: ANGLE_H = (hour % 12) * 30 + minute * 0.5.
	double angleHour = (state->hour % 12) * 30.0 + state->minute * 0.5;
	fprintf(output,
		"        <line x1=\"0\" y1=\"12\" x2=\"0\" y2=\"-52\" stroke=\"%s\" stroke-width=\"6\" stroke-linecap=\"round\" transform=\"rotate(%.2f)\"/>\n",
		handColor, angleHour);

	// Minute hand: ANGLE_M = minute * 6.
	double angleMinute = state->minute * 6.0;
	fprintf(output,
		"        <line x1=\"0\" y1=\"16\" x2=\"0\" y2=\"-72\" stroke=\"%s\" stroke-width=\"3\" stroke-linecap=\"round\" transform=\"rotate(%.2f)\"/>\n",
		handColor, angleMinute);

	// Central pivot.
	fprintf(output, "        <circle r=\"5\" fill=\"%s\"/>\n", handColor);

	fprintf(output, "      </svg>\n");
}

/** Emits the document head, embedded stylesheet and the opening container. */
static void generateDocumentHead(FILE * output) {
	fprintf(output,
		"<!DOCTYPE html>\n"
		"<html lang=\"es\">\n"
		"<head>\n"
		"  <meta charset=\"UTF-8\">\n"
		"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"  <title>Relojes</title>\n"
		"  <style>\n"
		"    body { margin: 0; background: #f0f0f0; }\n"
		"    .clocks-container {\n"
		"      display: flex;\n"
		"      flex-wrap: wrap;\n"
		"      gap: 20px;\n"
		"      padding: 20px;\n"
		"      justify-content: center;\n"
		"    }\n"
		"    .clock-wrapper {\n"
		"      display: flex;\n"
		"      flex-direction: column;\n"
		"      align-items: center;\n"
		"      gap: 8px;\n"
		"    }\n"
		"    .clock-label {\n"
		"      font-family: sans-serif;\n"
		"      font-size: 14px;\n"
		"    }\n"
		"  </style>\n"
		"</head>\n"
		"<body>\n"
		"  <div class=\"clocks-container\">\n"
	);
}

/** Closes the container and the document. */
static void generateDocumentTail(FILE * output) {
	fprintf(output,
		"  </div>\n"
		"</body>\n"
		"</html>\n"
	);
}

/* PUBLIC FUNCTIONS */

void generateHTML(ClockStateList * clocks) {
	logDebugging(_logger, "Generating HTML output...");
	FILE * output = stdout;
	generateDocumentHead(output);
	int emitted = 0;
	for (int i = 0; i < clocks->count; ++i) {
		ClockState * state = &clocks->clocks[i];
		if (!state->rendered) {
			continue;
		}
		fprintf(output, "    <div class=\"clock-wrapper\">\n");
		fprintf(output, "      <div class=\"clock-label\">%s</div>\n", state->name);
		generateClockSVG(state, output);
		fprintf(output, "    </div>\n");
		emitted++;
	}
	generateDocumentTail(output);
	fflush(output);
	logDebugging(_logger, "Generated %d clock(s).", emitted);
}
