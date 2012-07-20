#include "print_funcs.h"

#include <stdio.h>

void print_free_default(const char *timestr, double percent_free){
    fprintf(stdout, "%s: Free: %lf%%\n", timestr, percent_free);
}

//100%  free: [--------------------|]
//<100% free: [-------------------| ]
//<95%  free: [------------------|  ]
//<90%  free: [-----------------|   ]
//<85%  free: [----------------|    ]
//<80%  free: [---------------|     ]
//<75%  free: [--------------|      ]
//<70%  free: [-------------|       ]
//<65%  free: [------------|        ]
//<60%  free: [-----------|         ]
//<55%  free: [----------|          ]
//<50%  free: [---------|           ]
//<45%  free: [--------|            ]
//<40%  free: [-------|             ]
//<35%  free: [------|              ]
//<30%  free: [-----|               ]
//<25%  free: [----|                ]
//<20%  free: [---|                 ]
//<15%  free: [--|                  ]
//<10%  free: [-|                   ]
//<5%   free: [|                    ]
void print_free_visual(const char *timestr, double percent_free){
    double vis_step;
    char step = '-';

    fprintf(stdout, "%s: [", timestr);
    for(vis_step = 5.0; vis_step < 110.0; vis_step += 5.0){
        if(vis_step > percent_free)
            step = (step == '-') ? '|' : ' ';
        fprintf(stdout, "%c", step);
    }
    fprintf(stdout, "] (%lf%% Free)\n", percent_free);
}

