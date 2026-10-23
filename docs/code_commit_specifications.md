# Code Commit Specifications

## Programming Specifications
Comply with the C++ programming specifications of the CANN open repository: [C++ Programming Specifications of the CANN Community Open Repository](https://gitcode.com/cann/community/blob/master/contributor/coding-standards/C++%20Coding%20standards.md)
For items that are not included in some specifications, keep them consistent with the existing code style in the repository, including:
1. Operator code naming style: Use the camel case style for AscendC-related code. For CCE intrinsic-related variables, use lowercase letters and underscores (_). For classes, use the upper camel case style.
2. Operator host code naming style: camel case style.
3. Use lowercase letters and underscores (_) for directory and file names. The content must be consistent with the main classes or main interfaces in the file.

## MR Format
1. The commit format will be supplemented later. There is no requirement for now.
2. When submitting each MR, use the template and add the [WIP] label. After completing code check, pipeline compilation, pipeline DT, and clean code (excluding masked items), the code review conditions are met and the [WIP] label can be removed.
3. Unified title format: [bug/feature/task] Fix XX issue/Add XX feature/Rectify XX issue. For details, see the template.
4. The MR description should briefly describe the MR (requirement source, modified content, etc.). If interface changes are involved, the description must be emphasized and the changes must be synchronized with the upstream components.
5. Perform self-checks carefully according to the checklist.

## Code Review Merging
1. Enter two to four developers responsible for the code involved in the MR in the required field. Do not enter a large number of reviewers.
2. Contact the reviewer to review the MR. The reviewer needs to review each modification and cannot directly approve the MR.
3. The committer should centrally process MRs in two fixed time periods every day to ensure that all MRs are processed in time.
4. Unnecessary MRs should be closed in time. If a conflict occurs, handle it in the original MR. If a new MR needs to be created, close the original MR, and attach the link of the original MR to the description of the new MR and briefly explain the reason.
