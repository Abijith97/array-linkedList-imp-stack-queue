# Auto-Generated PR Descriptions Setup

This repository uses Claude AI to automatically generate comprehensive pull request descriptions whenever a new PR is created.

## How It Works

When anyone on the team creates a pull request, a GitHub Action automatically:
1. Fetches all the changes in the PR
2. Analyzes the code diff and commit history
3. Calls the Claude API to generate a detailed description
4. Updates the PR with the generated description

## Setup Instructions

### 1. Get an Anthropic API Key

1. Go to [console.anthropic.com](https://console.anthropic.com/)
2. Sign in or create an account
3. Navigate to API Keys section
4. Create a new API key
5. Copy the key (you won't be able to see it again)

### 2. Add API Key to GitHub Repository

1. Go to your GitHub repository
2. Click on **Settings** → **Secrets and variables** → **Actions**
3. Click **New repository secret**
4. Name: `ANTHROPIC_API_KEY`
5. Value: Paste your API key from step 1
6. Click **Add secret**

### 3. Push the GitHub Action Files

The following files need to be in your repository:
- `.github/workflows/auto-pr-description.yml` - The workflow configuration
- `.github/scripts/generate-pr-description.js` - The script that calls Claude API

These files are already created in this repository. Just commit and push them:

```bash
git add .github/
git commit -m "Add auto PR description generation workflow"
git push
```

### 4. Test It Out

1. Create a new branch with some changes
2. Push the branch to GitHub
3. Open a pull request
4. Watch as the description is automatically generated within seconds!

## What Gets Generated

The auto-generated description includes:
- **Summary**: What the PR does and why
- **Key Changes**: Bullet-pointed list of major changes
- **Test Plan**: Steps to verify the changes work correctly

## Customization

### Modify the Prompt

Edit `.github/scripts/generate-pr-description.js` and adjust the `prompt` variable to change what information is included or how it's formatted.

### Change the Model

The workflow uses `claude-sonnet-4-5-20250929` by default. You can change this to:
- `claude-opus-4-5-20251101` - More capable but slower and more expensive
- `claude-haiku-4-0-20250401` - Faster and cheaper but less detailed

Edit the model name in `.github/scripts/generate-pr-description.js`.

### Trigger on Different Events

The workflow currently triggers when a PR is `opened`. You can modify `.github/workflows/auto-pr-description.yml` to also trigger on:
- `reopened` - When a closed PR is reopened
- `edited` - When the PR title/body is edited

```yaml
on:
  pull_request:
    types: [opened, reopened]
```

## Troubleshooting

### The description isn't being generated

1. **Check the Actions tab** in your GitHub repository for error logs
2. **Verify the API key** is correctly set in repository secrets
3. **Check API quotas** at console.anthropic.com to ensure you haven't exceeded limits

### The description is too short/long

Adjust the `max_tokens` parameter in `generate-pr-description.js`:
- Increase for longer descriptions (e.g., 3000)
- Decrease for shorter descriptions (e.g., 1000)

### Rate limiting

If you have a high volume of PRs, consider:
- Using a cheaper/faster model (Haiku)
- Implementing rate limiting in the workflow
- Upgrading your Anthropic API plan

## Cost Estimation

Approximate costs per PR (as of January 2025):
- **Claude Sonnet 4.5**: ~$0.02-0.05 per PR
- **Claude Opus 4.5**: ~$0.10-0.20 per PR
- **Claude Haiku 4.0**: ~$0.001-0.005 per PR

Actual costs depend on the size of your changes.

## Security Notes

- The API key is stored as a GitHub secret and never exposed in logs
- The workflow only has read access to repository contents and write access to PRs
- Diff contents are sent to Anthropic's API (review their [privacy policy](https://www.anthropic.com/legal/privacy))

## Support

If you encounter issues:
1. Check the GitHub Actions logs for detailed error messages
2. Verify all setup steps were completed correctly
3. Consult the [Anthropic API documentation](https://docs.anthropic.com/)
