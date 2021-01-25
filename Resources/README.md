# Flash resource access

Flash storage is normally used for patches, but it also supports storing arbitrary data. This can be used for things like lookup tables with various math functions, presets, parameter settings, wavetables, and so on.

## A word of caution

A flash cell can take thousands of writes before becoming worn out and stopping functioning. OpenWare spreads writes over whole storage to distribute writes evenly and avoid overwriting a single cell too frequently. Writing resources occasionally is fine, but doing it at audio rate is a good way to leave you with malfunctioning hardware.

## Patch and resource slots

Every patch that you save occupies a slot on flash storage and has a name. The same is true about resources. Slot numbers for resources follow patch slots. Specific number can change in future firmware releases or for some devices, but for now we have the following order:

* 0: dynamic patch stored in memory
* 1 - 40: patches stored on flash
* 41 - 52: resources stored on flash

We will also address slots 41-52 as resource indices 0-11 and those numbers are relative to first patch storage. Resource indexes are expected to be unchanged even if resource is stored in a device with different number of slots.

## Creating new resouces

OwlProgram repository contains [Tools/makeresource.py](https://github.com/pingdynasty/OwlProgram/blob/master/Tools/makeresource.py) script. It can generate data in the same format that will be stored on flash.

Input for this script can be:
* a raw binary file that would be stored as is
* a C file header with lookup table as array, this would be converted to array of binary data
* a WAV file that would be converted to array of sample data (without WAV metadata)

Using this script may require specifying additional parameters to set input/output data format when it can't be determined automatically. You can see its included help with --help flag

## Loading resources

Just like patches, resources must be converted to MIDI SySex data format to be sent to device. This can be performed by calling

```
make RESOURCE=/path/to/resource SLOT=SLOT_NUMBER resource
```

Make sure to specify correct slot number. Note that this will overwrite anything previously stored in that slot.

## Freeing up space

Flash chip itself can only delete data in 128kb sectors, while our resources can be just a few bytes in size. That's why when you delete a resource (by overwriting or programmatically from a patch or UI), it's not being truly deleted. Instead we mark this data as deleted in file system and ignore it.

Eventually this will lead to storage becoming full. When this happens, we will copy all patches and resources that aren't deleted to memory and erase storage. Then we will copy data back to storage. Now it would be finally possible to utilize flash space previously used by the deleted data.

You can see total available size in storage and size used by patches and resources that aren't deleted in Magus menu.

## Access from patches

### C++

#### Serializing objects

"ResourceStorage.h" header includes all the necessary files:

* ResourceStorage - grants access to storage itself
* Resource - this class must include correct size attribute and buffer of that size must follow it. You're not expected to instantiate it manually, but it can be returned from some method of Storage class. Also, this is the base class for next two template classes.
* StorageResource<typename Payload> is the template used for reading resources stored on flash.
* MemoryResource<typename Payload> is the template uses an object allocated in memory as its payload.

You should instantiate one or both of those templates for each type of data that you want to access:

```
class TestData {
public:
    int value;
};

using TestMemoryResource = MemoryResource<TestData>;
using TestStorageResource = StorageResource<TestData>;
```

### Objects access

#### Reading data as an object

This can be done by calling ResourceStorage.getResource<> template with desired type as parameter and patch index or name:

```
const TestStorageResource* res = storage.getResource<TestData>(5);
const TestStorageResource* res = storage.getResource<TestData>("foo.bar");
```

Note that result is a pointer to resource object that can be NULL in case if no resource was found. Always check return for this!

There can be more than one resource with given name, in such case the first one would be used.

Finally, this method would not copy data to memory, but reference data from storage directly. This won't require using any RAM, but can only be used if storage support memory mapped access. The latter limitation is important, because OWL2 platform devices don't have such support. So you must copy resources from that storage using `.loadResources()` method.

#### Reading untyped data

Another way to read object data is by using the same method without template parameter.

```
const Resource* res = storage.getResource(5);
const Resource* res = storage.getResource("foo.bar");
```

This works the same way as method above, differing only in returned object pointer's type.

### Loading data to a preallocated object

Storage object can also be used for loading data from flash to memory. This is useful when you're loading an object from a storage that doesn't provide memory-mapped access or if you want to load data to RAM to lower access latency.

```
TestMemoryResource test_data;

storage.loadResource<TestMemoryResource>(5, test_data);
storage.loadResource<TestMemoryResource>("foo.bar", test_data);
```

Success status is return as boolean value.

#### Loading untyped data to a new buffer

The same method can be called with a different signature to allocate a new buffer and create an untyped resource that contains that data. This is particularly useful for accesing array contents, so additional parameters to
set length or offset for loaded data are available in this method:

```
Resource* res = storage.loadResource(5, 1024, 2048);
Resource* res = storage.loadResource("foo.bar", 1024, 2048);
```

The example above would load 1Kb with 2Kb offset. Note that result will be NULL if this operation fails.

#### Constructing array from resources

Array objects (FloatArray, IntArray, etc) include a few extra fields in addition to array data. Resource class provides `.asArray()` method to construct array of correct size that points to data stored in resource buffer:

```
FloatArray array = resource.asArray<FloatArray, float>();
```

This method can be used with objects of Resource class or any of its child classes.

#### Other operations

To delete resource, call `storage.deleteResource(uint8_t index);` with resource index as parameter. Number of resources can be read from `storage.getResourceCount();` method.

#### Sample patches

There's also a sample [resource listing patch](./ResListPatch.hpp) and [resource editing patch](./ResEditPatch.hpp) that can be used for quickly testsing all APIs.

## Magus UI

### Resource browser

A UI page titled "Data" is available with firmware v21.0 or newer. This allows you to browse all resources stored on device and see their slot numbers.

You can click on any resource to delete it unless it's write protected. Write protected resources are those with names starting with two underscores (`__`). Currently this is used for `__SETTINGS__` resource, but you can also write protect your own resources. This feature is only preventing accidentally deleting resources in UI - it's still possible to overwrite or delete them by MIDI or from your patches.

### RGB LEDs

You can add custom rainbow tables for Magus leds. This can be done to promote device diversity and chromatic fluidity (that is, for esthetic reasons).

Input LEDs will be loaded automatically if `Rainbow.in` resource is found and `Rainbow.out` is used output LEDs. You can edit existing [generator](https://github.com/pingdynasty/OpenWare/blob/master/Tools/rainbow.c) to create tables with custom colors. Then use makeresource.py script to convert them to resource, load it and enjoy your unique color scheme.

Default tables are read from "purple-blue-cyan.h" and "orange-red-pink.h" files in OpenWare repository. There are alternative color tables in "purple-indigo-violet-pink.h" and "crimson-orangered-gold-darkorange.h" that can be used to change colors without using generator.