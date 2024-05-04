{
  description = "Render engine for graphics course";

  inputs = {
    nixpkgs.follows = "khaser/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    khaser.url = "git+ssh://git@109.124.253.149/~git/nixos-config?ref=master";
  };

  outputs = { self, nixpkgs, flake-utils, khaser }:
    flake-utils.lib.eachDefaultSystem ( system:
    let
      pkgs = import nixpkgs { inherit system; };
      configured-vim = (khaser.lib.vim.override {
        extraPlugins = with pkgs.vimPlugins; [
          vim-cpp-enhanced-highlight
          YouCompleteMe
        ];
        extraRC = ''
          let g:cpp_class_scope_highlight = 1
          let g:cpp_member_variable_highlight = 1
          let g:cpp_class_decl_highlight = 1
          let g:cpp_posix_standard = 1
          let g:cpp_experimental_simple_template_highlight = 0
          let g:cpp_concepts_highlight = 1

          autocmd filetype cpp map <silent> <F1> :!export CMAKE_BUILD_TYPE=Debug && ./build.sh <CR>
          let &path.="include,src,${pkgs.glibc.dev}/include"
          let g:ycm_clangd_binary_path = '${pkgs.clang-tools_16}/bin/clangd'
        '';
      });
    in {
      devShell = pkgs.clangStdenv.mkDerivation {
        name = "cpp";

        nativeBuildInputs = with pkgs; [
          configured-vim
          cmake
          xxd
          gdb
          linuxKernel.packages.linux_zen.perf
          llvmPackages.openmp
        ];

      };
    });
}

