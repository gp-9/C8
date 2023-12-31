{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
    name = "chip8";

    nativeBuildInputs = with pkgs; [ 
      bear 
      clang 
      gcc 
      pkg-config 
    ];

    buildInputs = with pkgs; [ 
      raylib 
      valgrind 
      glfw
    ];

    shellHook = ''
        alias checkLeaks="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose"
    '';
}
