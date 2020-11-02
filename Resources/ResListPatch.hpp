#ifndef __RES_LIST_PATCH_HPP__
#define __RES_LIST_PATCH_HPP__

#include "Patch.h"
#include "ResourceStorage.h"

/*
 * This patch would output all found resources one by one by ID.
 * After that it would print if settings resource can be found by name.
 * So it tests those 2 service calls plus results of list resource call that is run on startup.
 */

class ResListPatch : public Patch {
public:
    ResListPatch(){
        sr = getSampleRate();
        index = 0;
    }

    void processAudio(AudioBuffer &buffer) {
        time += buffer.getSize();
        if (time >= sr){
            if (storage.getResourceCount()){                
                if (index == storage.getResourceCount()){
                    // Print info about settings
                    index = 0;
                    res = storage.getResource("__SETTINGS__");                    
                    if (res == NULL){
                        debugMessage("No settings");
                    }
                    else {
                        // Check if resource in first slot has the expected name used by settings
                        debugMessage("Settings match", int(!strcmp(res->name, "__SETTINGS__")));
                    }
                }
                else {
                    // Print info about every resource
                    res = storage.getResource(index);
                    if (res == NULL){
                        debugMessage("*EMPTY*", index);
                    }
                    else {
                        debugMessage(res->name, index);
                    }
                    index++;                
                }
            }
            else {
                // Oops, empty storage
                debugMessage("No resources");
            }
            time %= sr;
        }
    }

private:
    ResourceStorage storage;
    uint32_t time;
    uint32_t sr;
    uint8_t index;
    const Resource* res;
};

#endif