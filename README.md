DYLDSharedCache.hopperLoader
======

DYLD shared cache loader plugin for Hopper Disassembler. Besides cached libraries parsing this plugin attempts to fix `__LINKEDIT` segment. [Here](http://hopperapp.com/bugtracker/index.php?do=details&task_id=166)'s the reason why you might want to use that plugin and not the standard Hopper's DYLD plugin.


Installation
------------

Checkout or download the repository and use cmake/make:

    git clone https://github.com/0xc010d/DYLDSharedCache.hopperLoader.git DYLDSharedCache
    cd DYLDSharedCache
    git submodule init && git submodule update
    mkdir build && cd build
    cmake .. && make install

Restart Hopper Disassembler and you'll have a `DYLD Shared Cache (with fixed __LINKEDIT)` loader available when you choose to load `dyld_shared_cache*` file.


Enjoy!
------
