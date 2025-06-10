{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {

  buildInputs = with pkgs; [
    opencv4
  ];
  
  # nativeBuildInputs = with pkgs; [
  #   linuxPackages.perf
  #   graphviz
  #   python310Packages.gprof2dot
  #   valgrind
  # ]; 

  # shellHook = ''
  #   export NIX_SHELL_PROMPT=1
  #   echo "Entering dev shell..."
  # '';
}
