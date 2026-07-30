
#### 1.  **What should I do if the ascend-cla/no label is added to my PR?**

This label indicates that the commits of the PR are authored by one or more contributors who have not signed the CLA of the Ascend community. [Signing link](https://clasign.osinfra.cn/sign/68cbd4a3dbabc050b436cdd4) can be found in the PR comment area. Individual contributors should select "Sign Individual CLA", and enterprise administrators participating in contributions should select "Enterprise Administrator" to sign the CLA. CLA verification uses the commit information.

<table>
<tbody>
<tr>
<th>Scenario</th>
<th>Preference</th>
<th>Solution</th>
</tr>
<tr>
<td>Commit email identical to the GitCode submission email</td>
<td>This identical email address</td>
<td>Use this email address to sign the CLA.</td>
</tr>
<tr>
<td rowspan="2">Commit email different from the GitCode submission email</td>
<td>Commit email</td>
<td>Change the Gitcode submission email address to the commit email address. On the GitCode personal settings page, add the commit email address and set it as the submission email address. Then, sign the CLA.</td>
</tr>
<tr>
<td>GitCode submission email</td>
<td>On the local host where Git is running, run the `git config --global user.name **` and `git config --global user.email **` commands to change the commit email address of Git to the GitCode submission email address. Then, sign the CLA.</td>
</tr>
</tbody>
</table>



#### 2. **Why can't I fork the src-ascend/abc repository to my account?**

This issue typically occurs because a repository with the same name `abc` already exists under your personal account. For example, you may have previously forked a repository named `abc` from the Ascend organization. Since Gitee uses your personal account name plus the repository name for addressing, duplicate repository names under your personal account are not allowed.

Solution: Rename or change the path of the existing repository under your personal account, then fork the `src-ascend/abc` repository again.


#### 3. **Can I directly push code to non-protected branches as a non-maintainer contributor?**


Sorry, non-maintainer contributors cannot directly push code to any branches in the repositories, including both protected and unprotected branches.

The difference between protected and unprotected branches lies in whether maintainers can directly push code to them. On unprotected branches, maintainers have the permission to push code directly. However, on protected branches, even maintainers cannot directly push code. Instead, they must submit changes through PRs, which are then merged by openeuler-ci-bot.


#### 4. **Can maintainers directly push code to repositories?**


Maintainers can push code directly to unprotected branches, but not to protected branches.


#### 5. **What is the difference between directly pushing code to a repository and merging code via `/lgtm` or `/approve` comments?**


Using Git commands to directly push code to a repository bypasses necessary reviews, introducing risks. For example, when a file to upload is too large for a personal repository, you need push it directly to an unprotected branch in the repository, then merge the changes into a protected branch.

The code review process using the "/lgtm" or "/approve" comment ensures that at least one maintainer other than the author approves each code merging. Even if the author is a maintainer, another maintainer's approval is required before the code can be merged.

#### 6. **What commands can I use in the comments of Ascend community repositories and what are their functions?**


For details about the supported commands, see [Ascend Community Comment Commands](infra-command.md).

#### 7. **Why is CI build not triggered after I submit a PR?**


CI build will not be triggered in the following scenarios:

- Scenario 1: Due to network issues or system task scheduling issues, the webhook notification event sent from the code repository may not have reached the target service in time. In this case, you can re-trigger it by commenting `/retest` in the PR comment area.

- Scenario 2: The PR was submitted shortly after the code repository was created. At this point, the CI build project has not yet been created on the Jenkins server, so the CI build cannot be triggered, and commenting `/retest` will not work. In this case, please wait a moment for the system to automatically build the project.
