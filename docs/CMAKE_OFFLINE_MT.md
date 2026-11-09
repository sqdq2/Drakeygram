# Add Offline MT sources to CMake

In `Telegram/CMakeLists.txt`, next to the other translator sources, add:

```cmake
        ayu/features/translator/offline/offline_mt_packs.cpp
        ayu/features/translator/offline/offline_mt_packs.h
```

Place them after:

```cmake
        ayu/features/translator/implementations/google.h
```

Privacy: these units only **GET** model/catalog files. They never upload chat text.
