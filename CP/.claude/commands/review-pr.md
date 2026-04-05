Use the `reviewer` subagent to perform a thorough code review.

If $ARGUMENTS contains a PR number or branch name, run:
```
git diff main...$ARGUMENTS
git log main...$ARGUMENTS --oneline
```
to get the changeset.

If $ARGUMENTS is empty or "staged", review the current staged + unstaged changes:
```
git diff HEAD
git status
```

If $ARGUMENTS is a file path, review that specific file in the context of recent changes to it:
```
git log --oneline -10 -- $ARGUMENTS
git diff HEAD -- $ARGUMENTS
```

Pass everything to the reviewer agent along with the instruction: "Review this code change. You have no prior context — discover what you need by reading the diff and any files it touches. Apply the full review checklist and give a verdict."
