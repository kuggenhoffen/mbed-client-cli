/*
 * Copyright (c) 2018-2019, Pelion and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

// to achieve more identical behaviour with mbed device you can active ncurses
//#define EXAMPLE_USE_NCURSES 1

#ifndef EXAMPLE_USE_NCURSES
#define EXAMPLE_USE_NCURSES 0
#endif
#if EXAMPLE_USE_NCURSES == 1
#include <ncurses.h>
#endif
#include "mbed-trace/mbed_trace.h"
#include "ns_cmdline.h"

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

static void signal_handler(int signal)
{
    if (signal == SIGALRM) {
        cmd_ready(CMDLINE_RETCODE_SUCCESS);
        cmd_printf("Delayed command finished");
    }
}

// dummy command with some option
static int cmd_dummy(int argc, char *argv[])
{
    if (cmd_has_option(argc, argv, "o")) {
        cmd_printf("This is o option\r\n");
    } else {
        return CMDLINE_RETCODE_INVALID_PARAMETERS;
    }
    return CMDLINE_RETCODE_SUCCESS;
}
volatile bool running = true;
static int cmd_exit(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    running = false;
    return CMDLINE_RETCODE_SUCCESS;
}

static int cmd_delayed(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    alarm(2);
    return CMDLINE_RETCODE_EXCUTING_CONTINUE;
}

int main(void)
{
#if EXAMPLE_USE_NCURSES == 1
    initscr();    // Start curses mode
    raw();        // Line buffering disabled
    noecho();     // Don't echo() while we do getch
#endif

    signal(SIGALRM, signal_handler);

    // Initialize trace library
    mbed_trace_init();
    cmd_init(0);   // initialize cmdline with print function
    cmd_add("exit", cmd_exit, "exit shell", 0);
    cmd_add("dummy", cmd_dummy,
            "dummy command",
            "This is dummy command, which does not do anything except\n"
            "print text when o -option is given."); // add one dummy command
    cmd_add("delayed",
            cmd_delayed,
            "Command with delayed result.",
            "Command that can be used to test executing continue result.");

    cmd_init_screen();
    while (running) {
#if EXAMPLE_USE_NCURSES == 1
        int c = getch();
#else
        int c = getchar();
#endif
        switch (c) {
            case (CTRL('c')):
                running = false;
                break;
            case (EOF):
                break;
            default:
                cmd_char_input(c);
        }
    }
#if EXAMPLE_USE_NCURSES == 1
    endwin();
#endif
    cmd_free();
    return 0;
}
