# Automated Code Reviews with Claude

This repository uses Claude AI to automatically perform code reviews on all pull requests.

## How It Works

When anyone creates or updates a pull request, Claude automatically:
1. Analyzes all code changes in the PR
2. Reviews for:
   - Code quality and best practices
   - Potential bugs and edge cases
   - Security vulnerabilities
   - Performance issues
   - Test coverage
3. Posts a detailed review as a PR comment
4. Provides specific, actionable suggestions

## Setup Instructions

### Prerequisites

You should have already completed the PR description automation setup. If not, follow the steps in [PR_AUTOMATION_SETUP.md](PR_AUTOMATION_SETUP.md) to:
1. Get an Anthropic API key
2. Add it to GitHub secrets as `ANTHROPIC_API_KEY`

### Install the Code Review Automation

The files are already created. Just commit and push:

```bash
git add .github/
git commit -m "Add automated code review workflow"
git push
```

## What Gets Reviewed

Claude analyzes your code for:

### ⚠️ Critical Issues
- **Security vulnerabilities**: SQL injection, XSS, authentication flaws
- **Potential bugs**: Null pointer exceptions, race conditions, logic errors
- **Breaking changes**: API changes, backwards compatibility issues

### 💡 Suggestions
- **Code quality**: Readability, maintainability, DRY violations
- **Performance**: Inefficient algorithms, unnecessary operations
- **Best practices**: Language-specific idioms, design patterns
- **Testing**: Missing tests, edge cases not covered

### ✅ Positive Feedback
- Well-written code
- Good test coverage
- Proper error handling
- Clear documentation

## Review Triggers

The code review runs automatically when:
- A new PR is **opened**
- A PR is **updated** with new commits (synchronize)
- A closed PR is **reopened**

## Example Review Output

```markdown
## 🤖 Automated Code Review

### Code Quality ✅
The code is well-structured and follows C++ best practices. Good use of RAII principles.

### Potential Bugs ⚠️
1. In `array-stack.h:45` - No bounds checking before pop operation. This could cause undefined behavior if the stack is empty.
2. Missing null pointer check in `getData()` method.

### Suggestions 💡
1. Consider adding `[[nodiscard]]` attribute to getter methods to prevent ignoring return values
2. The `resize()` function could be optimized by using `std::move` instead of copy

### Security Concerns
No security issues detected.

### Testing
Good test coverage for basic operations. Consider adding tests for edge cases like popping from an empty stack.
```

## Customization

### Change Review Depth

Edit `.github/scripts/code-review.js` and adjust the `max_tokens` parameter:
- **Quick reviews**: 2000 tokens (cheaper, faster)
- **Standard reviews**: 4000 tokens (default)
- **Deep reviews**: 8000 tokens (thorough, more expensive)

### Modify Review Focus

Edit the `prompt` variable in `code-review.js` to focus on specific areas:
- Security-focused reviews
- Performance-focused reviews
- Style/formatting reviews
- Architecture reviews

Example for security-focused:
```javascript
const prompt = `Focus primarily on security vulnerabilities including:
- Authentication and authorization issues
- Input validation
- SQL injection, XSS, CSRF
- Cryptography misuse
...`;
```

### Change When Reviews Run

Edit `.github/workflows/auto-code-review.yml`:

```yaml
on:
  pull_request:
    types: [opened, synchronize]  # Remove 'reopened' if you don't want it
```

Or add specific path filters:
```yaml
on:
  pull_request:
    types: [opened, synchronize]
    paths:
      - 'src/**'
      - 'include/**'
```

### Use a Different Model

In `code-review.js`, change the model:
- `claude-opus-4-5-20251101` - Most thorough reviews (slower, expensive)
- `claude-sonnet-4-5-20250929` - Balanced (default, recommended)
- `claude-haiku-4-0-20250401` - Fast reviews (cheaper, less detailed)

## Cost Estimation

Approximate costs per code review:
- **Small PRs** (< 200 lines): $0.03-0.08
- **Medium PRs** (200-500 lines): $0.08-0.15
- **Large PRs** (> 500 lines): $0.15-0.30

Using Claude Sonnet 4.5. Costs vary based on diff size and complexity.

## Best Practices

### For Developers

1. **Don't ignore the reviews** - Claude often catches real issues
2. **Review is advisory** - You're still responsible for final decisions
3. **Ask questions** - If a suggestion is unclear, ask your team
4. **Update based on feedback** - Address critical issues before merging

### For Reviewers

1. **Claude augments human review** - It doesn't replace it
2. **Double-check critical issues** - Verify security/bug warnings
3. **Use it as a starting point** - Add your own insights and context
4. **Override when needed** - AI isn't always right

## Limitations

- **Context-limited**: Claude sees only the diff, not the entire codebase
- **No runtime analysis**: Can't catch issues that require execution
- **False positives**: May flag non-issues, use judgment
- **Language coverage**: Works best with popular languages (C++, Python, JS, etc.)

## Troubleshooting

### Reviews aren't posting

1. Check GitHub Actions logs for errors
2. Verify `ANTHROPIC_API_KEY` is set correctly
3. Ensure the workflow file is on your default branch

### Reviews are too generic

1. Increase `max_tokens` for more detailed analysis
2. Make PRs smaller (< 500 lines) for better reviews
3. Add more context in the PR description

### Too expensive

1. Switch to Claude Haiku model
2. Reduce `max_tokens` to 2000
3. Add path filters to only review certain files
4. Only trigger on `opened`, not `synchronize`

## Combining with PR Descriptions

Both automations work together:
1. **PR opened** → Description auto-generated + Code review posted
2. **PR updated** → Code review updates automatically
3. Both use the same `ANTHROPIC_API_KEY` secret

Total cost per PR: ~$0.05-0.35 depending on size

## Privacy & Security

- Code diffs are sent to Anthropic's API
- Review [Anthropic's privacy policy](https://www.anthropic.com/legal/privacy)
- API key is stored securely in GitHub secrets
- Consider if your code can be shared with third-party AI services
- For sensitive repos, consider manual reviews only

## Disable Reviews

To temporarily disable without deleting files:

```bash
# Rename the workflow file
mv .github/workflows/auto-code-review.yml .github/workflows/auto-code-review.yml.disabled
```

To re-enable:
```bash
mv .github/workflows/auto-code-review.yml.disabled .github/workflows/auto-code-review.yml
```

## Support

For issues:
1. Check GitHub Actions logs
2. Review the setup steps
3. Consult [Anthropic API docs](https://docs.anthropic.com/)
4. Verify your API credits at console.anthropic.com
