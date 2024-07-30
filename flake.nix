{
  description = "Javelin RP 2040";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system: 
  let 
    pkgs = import nixpkgs {
      inherit system;
    };
    local-pico-sdk = pkgs.pico-sdk.overrideDerivation (oldAttrs: {
      src = pkgs.fetchFromGitHub {
        owner = "raspberrypi";
        repo = "pico-sdk";
        rev = "1.5.1";
        fetchSubmodules = true;
        hash = "sha256-GY5jjJzaENL3ftuU5KpEZAmEZgyFRtLwGVg3W1e/4Ho=";
      };
    });
    # javelin-steno-rp2040 = (with pkgs; stdenv.mkDerivation {
    #   pname = "javelin-steno-rp2040";
    #   version = "main";
    #   nativeBuildInputs = [
    #     cmake
    #     clang
    #   ];
    #   buildPhase = "make -j $NIX_BUILD_CORES";
    # });
  in rec 
  {
    # defaultApp = flake-utils.lib.mkApp {
    #   drv = defaultPackage;
    # };
    # defaultPackage = javelin-steno-rp2040;
    devShell = pkgs.mkShell {
      buildInputs = with pkgs; [
        gcc-arm-embedded
        local-pico-sdk
        python3
        cmake
        udisks
        tio
      ];
      env = {
        PICO_SDK_PATH = "${local-pico-sdk}/lib/pico-sdk";
      };

    };
  }
  );
}
