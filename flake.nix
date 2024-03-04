{
  description = "Render engine for graphics course";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11";
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

          autocmd filetype cpp map <silent> <F1> :!./build.sh <CR>
          autocmd filetype cpp map <silent> <F2> :!./run.sh ./scenes/example.cmds ./artifacts/example.p6 <CR>
          let &path.="include,src,${pkgs.glibc.dev}/include"
          let g:ycm_clangd_binary_path = '${pkgs.clang-tools}/bin/clangd'
        '';
      });
    in {
      devShell = pkgs.mkShell {
        name = "cpp";

        nativeBuildInputs = with pkgs; [
          configured-vim
          gcc # compiler
          clang-tools # clangd(language server)
          cmake
          xxd
        ];

      };
    });
}

