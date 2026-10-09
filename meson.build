project(
  'tclap',
  'cpp',
  # Meson 0.56 cannot read a version file directly; use its Python runtime.
  version: run_command(
    find_program('python3', 'python'),
    '-c', 'import sys; print(open(sys.argv[1]).read().strip())',
    files('VERSION'),
    check: true,
  ).stdout().strip(),
  license: 'MIT',
  meson_version: '>=0.56.0',
)

# This is a minimum meson.build to use TCLAP.
#
# Community maintained / best effort integration for the Meson build system.
# Contact manuel.schmitz@liebherr.com for any questions.
#
# Examples and test cases are optional and can be activated with:
#
#   meson setup builddir -Dwith_meson_tests=true
#
# TODO: Full porting from autotools remains to be done.


# --
# Library (Main Dependency)
# --

tclap_dep = declare_dependency(
  include_directories: 'include',
)
meson.override_dependency('tclap_dep', tclap_dep)

# --
# Examples
# --

if (get_option('with_meson_tests'))
  subdir('examples')
endif

# --
# Tests
# --

if (get_option('with_meson_tests'))
  # TCLAP is known to have problems on Windows platforms.
  # Re-enable tests when TCLAP has solved those.
  if host_machine.system() != 'windows'
    subdir('tests')
  endif
endif


