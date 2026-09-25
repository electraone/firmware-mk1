/*
* Electra One MIDI Controller Firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

/**
 * @file DisplayBenchmark.h
 *
 * @brief Measures the throughput and the integrity of the SPI link to the
 *        display controller.
 *
 * The numbers this produces are the ones quoted when the SPI path is changed.
 * Every block it transfers is verified by reading it back, so a run reports
 * throughput and correctness together: a fast path that corrupts data is not
 * an improvement.
 *
 * Results are written through System::logger, which means they appear in the
 * behavioural trace without needing a channel of their own.
 *
 * Compiled in only when the firmware is built with BENCHMARK=1.
 */

#pragma once

#if defined(ELECTRA_BENCHMARK) && ELECTRA_BENCHMARK

namespace DisplayBenchmark {

/**
 * @brief Runs the whole suite and logs the results.
 *
 * Call once at boot, after the display has been initialised and before the
 * application starts painting, so that nothing else is competing for the bus.
 */
void run(void);

} // namespace DisplayBenchmark

#define DISPLAY_BENCHMARK_RUN() DisplayBenchmark::run()

#else

#define DISPLAY_BENCHMARK_RUN() ((void)0)

#endif
