# Contributing to png2dds

Thank you for being interested on contributing to png2dds! This project follows the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md).

## Questions and bug reports

You can use the [issue tracker](https://github.com/joseasoler/png2dds/issues) to ask questions and report bugs but do not forget to search (including closed issues) to see if your entry has been posted before.

## Contributions

The project and its contributions are currently managed using the [GitLab Flow](https://docs.gitlab.com/ee/topics/gitlab_flow.html). 

Before preparing and submitting a PR for a feature, create an issue on the [tracker](https://github.com/joseasoler/png2dds/issues) to allow for discussing and refining the idea before it is implemented.

Source code contributions must follow the [style guide](STYLE_GUIDE.md). The easiest way to comply with it is to set up [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and [clang-tidy](https://clang.llvm.org/extra/clang-tidy/). You should enable the PNG2DDS_CLANG_TIDY CMake option when you build your code and execute clang-format on all files you have modified before submitting your contribution.