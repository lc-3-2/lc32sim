{
  description = "LC-3.2 Simulator Development and Build Environment";

  inputs = {
    nixpkgs-23-05.url = "github:NixOS/nixpkgs/release-23.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs-23-05, flake-utils }@inputs :
    flake-utils.lib.eachSystem ["x86_64-linux"] (system:
    let
      name = "lc32sim-dev";
      pkgs = nixpkgs-23-05.legacyPackages.${system};
      inherit (pkgs) stdenv;

      buildInputs = [];
      nativeBuildInputs = [
        pkgs.cmake
        pkgs.ninja
        pkgs.boost
        pkgs.argparse
        pkgs.git
      ];
      propagatedBuildInputs = [
        pkgs.SDL2
      ];
    in {

      packages = rec {
        default = lc32sim;

        lc32sim = stdenv.mkDerivation {
          inherit name buildInputs nativeBuildInputs propagatedBuildInputs;

          src = self;

          configurePhase = ''
            runHook preConfigure
            cmake -G Ninja -B ./build/ -S . \
              -DCMAKE_INSTALL_PREFIX=$out -DCMAKE_BUILD_TYPE=Release \
              -DLC32SIM_SYSTEM_ARGPARSE=ON
            runHook postConfigure
          '';

          buildPhase = ''
            runHook preBuild
            ninja -v -C ./build/
            runHook postBuild
          '';
          installPhase = ''
            runHook preInstall
            ninja -v -C ./build/ install
            runHook postInstall
          '';
        };
      };

      devShells = rec {
        default = lc32sim;

        lc32sim = pkgs.mkShell {
          inherit name buildInputs nativeBuildInputs propagatedBuildInputs;

          shellHook = ''
            export PS1="(${name}) [\u@\h \W]\$ "
          '';
        };
      };
    });
}
