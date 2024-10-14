with import <nixpkgs> {};
mkShell.override { stdenv = llvmPackages_18.stdenv; } {
    buildInputs = [
        mold
    ];
    shellHook = ''
    '';
}
