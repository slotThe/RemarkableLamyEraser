{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  outputs = { self, nixpkgs }:
    let
      pkgs = import nixpkgs { system = "x86_64-linux"; };
      cross = pkgs.pkgsCross.remarkable2.pkgsStatic;
      clangd = pkgs.writeShellScriptBin "clangd" ''
        exec ${pkgs.clang-tools}/bin/clangd "--query-driver=${cross.stdenv.cc}/bin/*" "$@"
      '';
    in {
      packages.x86_64-linux.default = cross.stdenv.mkDerivation {
          pname = "remarkable-lamy-eraser";
          version = "2.2.0";
          src = pkgs.lib.fileset.toSource {
            root = ./.;
            fileset = pkgs.lib.fileset.unions [ ./Makefile ./sources ./headers ];
          };
          nativeBuildInputs = [ pkgs.gnumake ];
          strictDeps = true;
          allowedReferences = [ ];
          dontConfigure = true;
          enableParallelBuilding = true;
          installPhase = ''
            runHook preInstall
            install -Dm755 tmp/build/RemarkableLamyEraser "$out/bin/RemarkableLamyEraser"
            runHook postInstall
          '';
      };

      checks.x86_64-linux.timestamps = self.packages.x86_64-linux.default.overrideAttrs (old: {
          nativeBuildInputs = old.nativeBuildInputs ++ [ pkgs.qemu ];
          # Cross stdenv skips checkPhase even with doCheck enabled.
          postBuild = "make check RUNNER=${pkgs.qemu}/bin/qemu-arm";
      });

      devShells.x86_64-linux.default = cross.mkShell {
            inputsFrom = [ self.packages.x86_64-linux.default ];
            packages = [ pkgs.bear clangd pkgs.qemu ];
            shellHook = "export RUNNER=${pkgs.qemu}/bin/qemu-arm";
      };
    };
}
