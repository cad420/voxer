function(vm_target_dependency a b access)
  add_dependencies(${a} ${b})
  get_target_property(include ${b} INTERFACE_INCLUDE_DIRECTORIES)
  if (include)
    target_include_directories(${a} ${access} ${include})
  endif ()
  get_target_property(type ${b} TYPE)
  if (NOT "${type}" STREQUAL "INTERFACE_LIBRARY")
    if ("${type}" STREQUAL "STATIC_LIBRARY")
      target_link_libraries(${a} ${b})
    endif ()
    get_target_property(static ${b} LINK_LIBRARIES)
    if (static)
      target_link_libraries(${a} ${static})
    endif ()
  endif ()
endfunction()

function(vm_external_module)

  function(checkout_external_module repo_url tag)
    string(REPLACE "/" ";" repo_url_arr ${repo_url})
    list(GET repo_url_arr -1 repo_name)
    find_package(Git)
    if (NOT GIT_FOUND)
      message(FATAL_ERROR "Git not found")
    endif ()
    set(repo_dir "${CMAKE_BINARY_DIR}/external/${repo_name}")
    set(bin_dir "${CMAKE_BINARY_DIR}/external_build/${repo_name}")
    if (NOT EXISTS ${repo_dir})
      execute_process(COMMAND
          ${GIT_EXECUTABLE} clone "${repo_url}" --recursive ${repo_dir}
          )
    else ()
      execute_process(COMMAND
          git config --get remote.origin.url
          WORKING_DIRECTORY ${repo_dir}
          OUTPUT_VARIABLE remote_url
          )
      string(STRIP "${remote_url}" remote_url)
      #if(NOT "${remote_url}" STREQUAL "${repo_url}")
      #  message(FATAL_ERROR "remote url in cache dir does not match, remote url is ${remote_url} but required url is ${repo_url}")
      #endif()
    endif ()
    execute_process(COMMAND
        ${GIT_EXECUTABLE} pull --recurse-submodules WORKING_DIRECTORY ${repo_dir}
        )
    execute_process(COMMAND
        ${GIT_EXECUTABLE} checkout "${tag}" WORKING_DIRECTORY ${repo_dir}
        )
    add_subdirectory(${repo_dir} ${bin_dir})
  endfunction()

  set(options OPTIONAL FAST)
  set(oneValueArgs GIT_REPOSITORY GIT_TAG)
  set(multiValueArgs)
  cmake_parse_arguments(AEM
      "${options}"
      "${oneValueArgs}"
      "${multiValueArgs}"
      ${ARGN}
      )
  if (NOT AEM_GIT_TAG)
    set(AEM_GIT_TAG master)
  endif ()
  if (NOT AEM_GIT_REPOSITORY)
    message(FATAL_ERROR "GIT_REPOSITORY must be specified")
  endif ()
  set(repo_url ${AEM_GIT_REPOSITORY})
  if (repo_url MATCHES "https://github.com/.*.git$")
    string(REGEX REPLACE "\.git$" "" repo_url ${repo_url})
  endif ()
  get_property(tag GLOBAL PROPERTY "${repo_url}_TAG")
  if (tag)
    if (NOT "${tag}" STREQUAL "${AEM_GIT_TAG}")
      message(FATAL_ERROR
          "module ${repo_url} has conflict tags: "
          "${tag} <-> ${AEM_GIT_TAG}"
          )
    endif ()
  else ()
    set(tag ${AEM_GIT_TAG})
    message(${repo_url} " -> ${tag}")
    checkout_external_module(${repo_url} ${tag})
  endif ()
  set_property(GLOBAL PROPERTY "${repo_url}_TAG" "${tag}")
endfunction()
