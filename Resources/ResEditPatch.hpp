#ifndef __RES_EDIT_PATCH__
#define __RES_EDIT_PATCH__

#include "Patch.h"
#include "ResourceStorage.h"

#define TEST_RESOURCE_INDEX 5 // <- chosen by an engineering method of dice roll, no less
//#define ALLOW_OVERWRITE
// Uncomement the above to proceed with testing even if there's something in test slot
#define TOKEN1              0xABADDA7A
#define TOKEN2              0xBE7AC0DE

enum State {
    ST_WRITE_RES,
    ST_READ_RAW,
    ST_READ_RES,
    ST_EDIT,
    ST_CONFIRM,
    ST_DELETE,
    ST_DONE,
    ST_ERROR,
};

class TestData {
public:
    uint8_t the_byte_of_death; // Let's have this to ensure unaligned objects would work correctly
    int value;
};

using TestMemoryResource = MemoryResource<TestData>;
using TestStorageResource = StorageResource<TestData>;

class ResEditPatch : public Patch {
public:
    ResEditPatch(){
        state = ST_WRITE_RES;
    }
    void processAudio(AudioBuffer &buffer) {
        switch(state){
        case ST_WRITE_RES: {
            #ifndef ALLOW_OVERWRITE
            if (storage.getResource(TEST_RESOURCE_INDEX) != NULL) {
                // We don't want to proceed with testing if there's an existing resource
                // in test slot. So change slot number or delete resource manually or
                // enable the ALLOW_OVERWRITE flag
                debugMessage("Occupied test resource #", TEST_RESOURCE_INDEX);
                state = ST_ERROR;
                break;
            }
            #endif
            // Write test data
            TestMemoryResource res;
            res.payload.value = TOKEN1;
            res.setName("test1");
            if (storage.storeResource(TEST_RESOURCE_INDEX, res)){
                debugMessage("Write1 OK");
                state = ST_READ_RAW;
            }
            else {
                debugMessage("Write1 FAIL");
                state = ST_ERROR;
            }
            break;
        }
        case ST_READ_RAW: {
            // Read back untyped data
            const Resource* res = storage.getResource(TEST_RESOURCE_INDEX);
            if (res == NULL){
                debugMessage("Read1 FAIL");
            }
            else {
                if (*((int*)res->getData() + 1) == TOKEN1){
                    debugMessage("Read1 OK");
                    state = ST_READ_RES;
                }
                else {
                    debugMessage("Read1 invalid:", *(int*)res->getData());
                    state = ST_ERROR;

                }
            }
            break;
        }
        case ST_READ_RES: {
            // Read back typed data
            const TestStorageResource* res = storage.getResource<TestData>(TEST_RESOURCE_INDEX);
            if (res == NULL){
                debugMessage("Read2 FAIL");
            }
            else {
                if (res->getPayload().value == TOKEN1){
                    debugMessage("Read2 OK");
                    state = ST_EDIT;
                }
                else {
                    debugMessage("Read2 invalid:", res->getPayload().value);
                    state = ST_ERROR;
                }
            }
            break;
        }
        case ST_EDIT: {
            // Read storage resource
            const TestStorageResource* res1 = storage.getResource<TestData>(TEST_RESOURCE_INDEX);
            // Copy storage resource to memory resource
            TestMemoryResource res2;
            res2 = *res1;
            // Edit and store updated resource
            res2.setName("test2");
            res2.payload.value = TOKEN2;
            if (storage.storeResource(TEST_RESOURCE_INDEX, res2)){
                debugMessage("Write2 OK");
                state = ST_CONFIRM;
            }
            else {
                debugMessage("Write2 FAIL");
                state = ST_ERROR;
            }
            break;
        }
        case ST_CONFIRM: {
            const TestStorageResource* res = storage.getResource<TestData>(TEST_RESOURCE_INDEX);
            if (res == NULL){
                debugMessage("Read3 FAIL");
            }
            else {
                if (res->getPayload().value == TOKEN2){
                    debugMessage("Read3 OK");
                    state = ST_DELETE;
                }
                else {
                    debugMessage("Read3 invalid:", *(int*)res->getData());
                    state = ST_ERROR;
                }
            }
            break;
        }
        case ST_DELETE: {
            storage.deleteResource(TEST_RESOURCE_INDEX);
            if (storage.getResource(TEST_RESOURCE_INDEX) == NULL){
                debugMessage("Delete OK");
                state = ST_DONE;
            }
            else {
                debugMessage("Delete fail");
                state = ST_ERROR;
            }
            break;
        }
        case ST_DONE:
            debugMessage("We're good!");
            break;
        case ST_ERROR:
            // Don't do anything and just keep last message visible
            break;
        }
    }

private:
    ResourceStorage storage;
    State state;
};

#endif