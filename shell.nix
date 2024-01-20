with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_16.stdenv; } {
    buildInputs = [
        mold
    ];
    shellHook = ''
    '';
}
