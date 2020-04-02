include_guard()

option(VMCORE_SHARED_LIBRARY "" OFF)
vm_external_module(GIT_REPOSITORY https://github.com/cad420/VMCore.git GIT_TAG master)
