




	if (argc != 2)
	{
		fputs("? MAKEGROUP: Command error, usage is:\n"
				"               makegroup ipaddress\n". stdout);
		return (1);
	}
	if ((grpfile = openf("xoscfg:group.def", "w")) == NULL ||
			fprintf(grpfile, "USER UDP0: [%s] 3002\n", argv[1]) < 0 ||
			closef(grpfile) < 0)
		femsg2(prgname, "Error creating group definition file xoscfg:group.def",
				-errno, NULL);
	printf("MAKEGROUP: Group definition file xoscfg:group.def created for %s\n",
			argv[1]);
	return (0);
}
