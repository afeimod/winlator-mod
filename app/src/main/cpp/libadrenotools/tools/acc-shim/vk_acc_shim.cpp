#include <stdint.h>
#include <dlfcn.h>
#include <string.h>
#include <android/log.h>

thread_local int satisfy_driver_emutls;

// Check libllvm-glnext.so in older drivers for a full list of these, newer ones are slightly more awkward due to the split
enum class OptionType : uint32_t {
    ArgsStringList = 1,
    QcArgsString = 4,
    OptLevelInt = 0x101,
    NoOptsInt = 0x10D,
};

struct StringListVal {
    uint32_t len;
    const char **entry;
};

struct Option
{
    OptionType type;
    // PAD
    union {
        bool boolVal;
        int intVal;
        float floatVal;
        StringListVal *strVals;
        const char *strVal;
    };
};


struct OptionsSet
{
  Option *options;
  uint32_t num;
};


int CompileHook(void *state, OptionsSet *opts);

decltype(CompileHook) *OrigCompile{};

int CompileHook(void *state, OptionsSet *opts) {
    constexpr size_t ReplacementOptsNum = 201;

    Option replacementOpts[ReplacementOptsNum];
    uint32_t patchedOptNum{};

    // Insert your desired args here - results will be written to stderr
    const char *argv[] = {"-help", "-help-hidden", "", NULL };
    StringListVal listVal{
        .len = 3,
        .entry = argv,
    };
 
    replacementOpts[patchedOptNum++] = {
        .type = OptionType::ArgsStringList,
        .strVals = &listVal
    };

    const char *opt ="Verbose=true Quiet=false";
    replacementOpts[patchedOptNum++] = {
        .type = OptionType::QcArgsString,
        .strVal = opt
    };

    if (opts->num + patchedOptNum > ReplacementOptsNum)
        __builtin_trap();

    memcpy(&replacementOpts[patchedOptNum], opts->options, sizeof(Option) * opts->num);

    OptionsSet optList{
        .options = replacementOpts,
        .num = opts->num + patchedOptNum
    };

    return OrigCompile(state, &optList);
}


struct LlvmInterface {
  uint32_t unk0[3];
  // PAD
  void *makeContext;
  void *delContext;
  decltype(CompileHook) *compile;
  void *unk;
};

extern "C" int LoadACC(LlvmInterface *impl);

extern "C" __attribute__((visibility("default"))) int LoadACC(LlvmInterface *impl) {
    void *hnd = dlopen("notreal-glnext.so", RTLD_NOW);

    auto origLoadACC = reinterpret_cast<decltype(LoadACC) *>(dlsym(hnd, "LoadACC"));
    if (!origLoadACC)
        __builtin_trap();

    int ret = origLoadACC(impl);

    OrigCompile = impl->compile;
    impl->compile = &CompileHook;

    return ret;
}
