#include "main.h"
#include "chef.h"
#include "table.h"

void table_init(Table* t, int i, Chef** chefs, int num_chefs) {
    t->id = i;
    // tid assigned by pthread_create

    t->state = TABLE_ST_PREPARING;
    
    t->chefs = chefs;
    t->num_chefs = num_chefs;

    pthread_mutex_init(&(t->protect), NULL);
    pthread_cond_init(&(t->cv_foodie), NULL);
}

void* table_run(void* args) {
    Table* self = (Table*)args;

    while(true) {
        // OVERALL SERVING CYCLE

        pthread_mutex_lock(&(self->protect));
        // just to protect state change from Foodies
        self->state = TABLE_ST_PREPARING;
        pthread_mutex_unlock(&(self->protect));

        while(true) {
            // JUST GET A VESSEL
            int i = rand() % self->num_chefs;
            Chef* chef = self->chefs[i];
            pthread_mutex_lock(&(chef->protect));

            if (chef->left_vessels > 0) {
                chef->left_vessels--;
                self->left_vessel_cap = chef->vessel_cap;
                pthread_cond_signal(&(chef->cv_table));
                pthread_mutex_unlock(&(chef->protect));
                break;
            }

            pthread_cond_signal(&(chef->cv_table));
            pthread_mutex_unlock(&(chef->protect));
        }

        // NOW WE HAVE A VESSEL
        
        while (true) {
            // THIS IS THE SLOT GENERATION LOOP
            
            if (self->left_vessel_cap <= 0) { // NOBODY ELSE USES LEFT_VESSEL_CAPACITY
                self->left_vessel_cap = 0;
                break;
            }

            pthread_mutex_lock(&(self->protect));
            // just to protect state change and left_slots from Foodies
            self->state = TABLE_ST_INTERMEDIATE;
            
            self->total_slots = TABLE_SLOTS_OFFSET + rand() % TABLE_SLOTS_LIMIT;
            if (self->total_slots > self->left_vessel_cap) {
                self->total_slots = self->left_vessel_cap;
            }

            self->left_slots = self->total_slots;

            self->left_vessel_cap = self->left_vessel_cap - self->total_slots;

            self->state = TABLE_ST_SERVING;

            ready_to_serve_table(self);
        }
    }
}

void ready_to_serve_table(Table* self) {
    while (true) {
        if (self->left_slots <= 0) {
            break;
        } else {
            pthread_cond_wait(&(self->cv_foodie), &(self->protect));
        }
    }

    self->left_slots = 0;
    self->state = TABLE_ST_INTERMEDIATE;
    // TODO: print for all students here
    pthread_mutex_unlock(&(self->protect));
}