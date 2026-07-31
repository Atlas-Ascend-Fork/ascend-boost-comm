## Understanding Code of Conduct
Ascend Boost Comm is a CANN open project. Before contributing, please read the [CANN Open Project Code of Conduct](contributors/code-of-conduct.md). All your activities in the Ascend Boost Comm project (including but not limited to posting comments, submitting issues, and publishing wikis) must comply with this code of conduct.

## Signing CLA

You must sign a Contributor License Agreement (CLA) before you can contribute to the community.

Please choose the appropriate CLA based on your participation status: Corporate, Corporate Contributor, Individual, or Enterprise Admin. Click [here](https://clasign.osinfra.cn/sign/68cbd4a3dbabc050b436cdd4) to sign.

- Corporate CLA: For contributions made on behalf of a corporate. A representative of the corporate should sign this CLA, typically an administrator.
- Corporate Contributor CLA: If you are an employee of a corporate that has already signed a Corporate CLA, apply to sign this Corporate Contributor CLA. Select your corporate on the application page. The application will be reviewed and approved by the enterprise administrator, after which you can participate in contributions.
- Individual CLA: For contributions made as an individual who is not a corporate employee.
- Enterprise Administrator CLA: If you are an enterprise administrator, sign this CLA. Enterprise administrators have the authority to review and approve applications for signing the Corporate Contributor CLA and manage personnel.


## Contributing

After signing the CLA, you can begin your contribution journey. Your contribution can be in many ways and will be highly valued.

All discovered issues or new ideas you wish to contribute can be discussed and tracked through [Issues](#submitting-and-handling-issues), and can be closed after you [contribute code](#contributing-code) via pull requests.

> 📝 **Note**
>
> - For details about the GitCode workflow, see [Gitcode Workflow Description](contributors/gitcode-workflow.md).
> - If you encounter any problems when submitting a PR, refer to [FAQ](contributors/infra-faqs.md).

### Contribution Categories

- Code bug fixing

  If you discover a bug within this repository and wish to fix it, create a new issue in the repository for feedback and tracking.

  You can follow the instructions in [Submitting and Handling Issues](#submitting-and-handling-issues) to create a `Bug-Report` type issue describing the bug.
  Enter `/assign` or `/assign @yourself` in the comment box to assign the issue to you for processing.

- Document correction

  If you discover errors in operator documentation within the repository, create a new issue in the repository for feedback and correction.

  You can follow the instructions in [Submitting and Handling Issues](#submitting-and-handling-issues) to create a `Documentation` type issue to point out the errors.
  Enter `/assign` or `/assign @yourself` in the comment box to assign the issue to you for correcting the documentation.

- Resolving Issues

  If you have a solution for someone else's Issue, please share it in the comments to help the community.

  If the issue requires code modification, you can enter `/assign` or `/assign @yourself` in the comment box to assign the issue to yourself for assisted handling.

### Submitting and Handling Issues

- Finding the issue list

  In the [Ascend Boost Comm](https://gitcode.com/cann/ascend-boost-comm) project homepage on GitCode, click `Issues` to find the issue list.

- Submitting an issue

  If you want to report a bug, submit a requirement, or send your feedback to the community, please submit an issue.

  For details, see [Issue Submission Guide](contributors/issue-submit.md).

- Participating in issue discussions

  Each issue is open for developers to communicate and discuss. If you are interested, you can share your thoughts in comments.

- Finding an issue you want to handle

  If you want to handle one of the issues, you can assign it to yourself. You only need to enter `/assign` or `/assign @yourself` in the comment box. The bot will assign the issue to you and your name will be displayed in the assignee list.

### Contributing Code

1. CANN Development Environment Setup

   If you want to contribute code, you need to set up the CANN development environment. For details, see [Environment Setup](../README_en.md#3-environment-setup).

2. Ascend Boost Comm Development Precautions

   (1) For details about the environment and tool requirements for code contribution, see [Tool Version Requirements and Installation](../README_en.md#tool-version-requirements-and-installation).

   (2) The Ascend Boost Comm software code complies with the CANN Open Software License Agreement Version 2.0. For details about the agreement, see [LICENSE](../LICENSE). If you contribute code to the Ascend Boost Comm source code repository, comply with this agreement.

     Add the following statement to the header of the new source code files such as `.cpp`, `.cc`, and `.h`:

     ```
     /**
      * Copyright (c) [Name of the copyright owner]. 2025. All rights reserved.
      * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
      * CANN Open Software License Agreement Version 2.0 (the "License").
      * Please refer to the License for details. You may not use this file except in compliance with the License.
      * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
      * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
      * See LICENSE in the root of the software repository for the full text of the License.
      */
     ```

     Add the following statement to the header of the new files such as `.py` and `.sh`:

     ```
     # Copyright (c) [Name of the copyright owner]. 2025. All rights reserved.
     # This program is free software, you can redistribute it and/or modify it under the terms and conditions of
     # CANN Open Software License Agreement Version 2.0 (the "License").
     # Please refer to the License for details. You may not use this file except in compliance with the License.
     # THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
     # INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
     # See LICENSE in the root of the software repository for the full text of the License.
     # ================================================================================================================
     ```

    - If you are contributing as an individual and you own the copyright to the contributed content, replace [Name of the copyright owner] in the first line with your name.
    - If you are contributing on behalf of your employer, or if your employer owns the copyright to your contributed content, replace [Name of the copyright owner] in the first line with your employer's name.

      If you have any doubts about the copyright ownership of your contribution, please consult your legal advisor or your employer's legal team.

    - The year `2025` in the first line is the year you created or modified the file. Modify it according to the actual year.

3. Understand the code download and contribution process.
   (1) Before developing code, fork the Ascend Boost Comm repository to your own repository and then download the repository to your local computer. Then, modify, build, and verify the code in the local branch.
   (2) After the code meets the contribution requirements, submit a PR to contribute the code to the Ascend Boost Comm. You can find the submitted pull request in the [PR List](https://gitcode.com/cann/ascend-boost-comm/pulls).
   (3) In the comment area of a submitted pull request, comment `compile` to trigger the build.
   (4) Monitor the CI test result. If the test fails, modify the local code as prompted. If the test is passed, the PR will be assigned to a committer for review. Pay attention to the committer's review comments.
   (5) If your PR is approved, the code will be merged into the Ascend Boost Comm source code repository.
