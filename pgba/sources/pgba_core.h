//
// Created by Oliver on 04/25/2024
//

#ifndef PGBA_CORE_H
#define PGBA_CORE_H

int  mgba_init();
void mgba_fini();

int  mgba_start(const ss_api::Game &game);
void mgba_stop();

void mgbp_step();

#endif