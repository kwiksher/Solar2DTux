local params =
	{
		platform='linux',
		arch    = 'arm', -- armhf, arm64, x86_64, amd64
		appName='hello',
		appVersion='1.0',
		dstPath='.',
		projectPath='./HelloWorld',
		linuxtemplate='../linux/bin/Resources/template_arm.tgz',
	}
	return params