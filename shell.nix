with import <nixpkgs> {};
mkShell {
    buildInputs = [
        mold
        python3Packages.flake8
        shellcheck
    ];
    shellHook = ''
        . .shellhook
    '';
}
