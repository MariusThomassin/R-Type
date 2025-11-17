# Self-hosted runner (quick guide)

If GitHub Actions jobs are blocked due to billing/spending limits, using a self-hosted runner is a quick workaround. The steps below show how to register a runner on a Linux machine (your dev machine or a small VPS).

1) Create a runner in GitHub
- Open the repo on GitHub -> Settings -> Actions -> Runners -> New self-hosted runner
- Choose Linux and follow the instructions: you'll get a download URL and a token.

2) On your Linux machine (example commands)

```bash
# create a folder and download the runner (replace URL with the one from GitHub)
mkdir actions-runner && cd actions-runner
curl -O -L "https://github.com/actions/runner/releases/download/v2.303.0/actions-runner-linux-x64-2.303.0.tar.gz"
tar xzf ./actions-runner-linux-x64-2.303.0.tar.gz

# configure the runner (replace URL and TOKEN shown by GitHub when creating the runner)
./config.sh --url https://github.com/<owner>/<repo> --token <TOKEN> --unattended --labels self-hosted,linux

# run the runner in background (example uses systemd — see GitHub docs for details)
./run.sh
```

3) Run as a service (optional)
- Follow the instructions printed by `./svc.sh install` / `./svc.sh start` or create a systemd unit per GitHub docs.

Notes
- Self-hosted runners run jobs with the permissions of the machine user. Be careful with security and don't register a runner on a shared or untrusted host.
- Runners need network access to GitHub. Keep the runner up to date.
