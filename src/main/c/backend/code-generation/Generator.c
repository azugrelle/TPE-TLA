#include "Generator.h"
#include <ctype.h>
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

/** ANGLE_H = (hour % 12) * 30 + minute * 0.5. */
static double hourAngle(const ClockState * state) {
	return (state->hour % 12) * 30.0 + state->minute * 0.5;
}

/** ANGLE_M = minute * 6. */
static double minuteAngle(const ClockState * state) {
	return state->minute * 6.0;
}

/**
 * Builds a CSS-identifier-safe, document-unique id for a clock, written into
 * `buf`. The leading index guarantees uniqueness even if two clocks share a
 * name (or sanitize to the same string); non-alphanumeric characters in the
 * name become '_'.
 */
static void buildClockId(const ClockState * state, int index, char * buf, size_t size) {
	int pos = snprintf(buf, size, "clk%d-", index);
	if (pos < 0) {
		buf[0] = '\0';
		return;
	}
	for (const char * p = state->name; *p != '\0' && pos < (int) size - 1; ++p) {
		unsigned char c = (unsigned char) *p;
		buf[pos++] = isalnum(c) ? (char) c : '_';
	}
	buf[pos] = '\0';
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

/** Returns a foreground color that is readable on top of `bg`. */
static Color contrastColor(Color bg) {
	return (bg == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
}

/** Emits the 12 clock-face numbers, Arabic or Roman according to the style. */
static void generateNumbers(ClockState * state, FILE * output) {
	const char * fill = colorToHex(contrastColor(state->style.bgColor));
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
static void generateClockSVG(ClockState * state, const char * clockId, FILE * output) {
	const char * bgColor = colorToHex(state->style.bgColor);
	const char * borderColor = colorToHex(state->style.borderColor);
	Color resolvedHand = (state->style.handColor == state->style.bgColor)
		? contrastColor(state->style.bgColor)
		: state->style.handColor;
	const char * handColor = colorToHex(resolvedHand);

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

	int roman = (state->style.numbers == NUMBER_ROMAN);

	// Hour hand: ANGLE_H = (hour % 12) * 30 + minute * 0.5, one turn / 12h.
	fprintf(output,
		"        <line x1=\"0\" y1=\"12\" x2=\"0\" y2=\"%d\" stroke=\"%s\" stroke-width=\"6\" stroke-linecap=\"round\" class=\"hand-%s-hour\"/>\n",
		roman ? -42 : -52, handColor, clockId);

	// Minute hand: ANGLE_M = minute * 6, one turn / hour.
	fprintf(output,
		"        <line x1=\"0\" y1=\"16\" x2=\"0\" y2=\"%d\" stroke=\"%s\" stroke-width=\"3\" stroke-linecap=\"round\" class=\"hand-%s-minute\"/>\n",
		roman ? -54 : -64, handColor, clockId);

	// Second hand starts at 0. One turn / minute, fixed red for visibility.
	fprintf(output,
		"        <line x1=\"0\" y1=\"20\" x2=\"0\" y2=\"-80\" stroke=\"#E74C3C\" stroke-width=\"1.5\" stroke-linecap=\"round\" class=\"hand-%s-second\"/>\n",
		clockId);

	// Numbers drawn after hands so they always render on top.
	generateNumbers(state, output);

	// Central pivot.
	fprintf(output, "        <circle r=\"5\" fill=\"%s\"/>\n", handColor);

	fprintf(output, "      </svg>\n");
}

/** Emits the document head, embedded stylesheet and the opening container. */
static void generateDocumentHead(ClockStateList * clocks, FILE * output) {
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
	);

	for (int i = 0; i < clocks->count; ++i) {
		ClockState * state = &clocks->clocks[i];
		if (!state->rendered) continue;
		char id[128];
		buildClockId(state, i, id, sizeof(id));
		double ah = hourAngle(state);
		double am = minuteAngle(state);
		fprintf(output,
			"    @keyframes rotate-%s-hour   { from { transform: rotate(%.2fdeg); } to { transform: rotate(%.2fdeg); } }\n"
			"    @keyframes rotate-%s-minute { from { transform: rotate(%.2fdeg); } to { transform: rotate(%.2fdeg); } }\n"
			"    @keyframes rotate-%s-second { from { transform: rotate(0deg); } to { transform: rotate(360deg); } }\n"
			"    .hand-%s-hour   { transform-origin: 0 0; animation: rotate-%s-hour   43200s linear infinite; }\n"
			"    .hand-%s-minute { transform-origin: 0 0; animation: rotate-%s-minute  3600s linear infinite; }\n"
			"    .hand-%s-second { transform-origin: 0 0; animation: rotate-%s-second    60s linear infinite; }\n",
			id, ah, ah + 360.0,
			id, am, am + 360.0,
			id,
			id, id,
			id, id,
			id, id
		);
	}

	fprintf(output,
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
	generateDocumentHead(clocks, output);
	int emitted = 0;
	for (int i = 0; i < clocks->count; ++i) {
		ClockState * state = &clocks->clocks[i];
		if (!state->rendered) {
			continue;
		}
		char clockId[128];
		buildClockId(state, i, clockId, sizeof(clockId));
		fprintf(output, "    <div class=\"clock-wrapper\">\n");
		fprintf(output, "      <div class=\"clock-label\">%s</div>\n", state->name);
		generateClockSVG(state, clockId, output);
		fprintf(output, "    </div>\n");
		emitted++;
	}
	generateDocumentTail(output);
	fflush(output);
	logDebugging(_logger, "Generated %d clock(s).", emitted);
}
