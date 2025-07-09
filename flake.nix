{
  description = "Vasciipp Flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {

        # Package definition
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "vasciipp";
          version = "0.1.0";
          src = ./.;
          nativeBuildInputs = with pkgs; [ cmake ];
          buildInputs = with pkgs; [ opencv ];
        };

        apps.${system}.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/vasciipp";
        };

      }
    );
}
