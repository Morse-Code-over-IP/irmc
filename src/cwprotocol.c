	id_packet.command = DAT;
	id_packet.length = 492;
	snprintf(id_packet.id, 128, id, "%s");
	id_packet.sequence = 0;
	id_packet.n = 0;
	snprintf(id_packet.status, 128, "irmc v0.02");
	id_packet.a21 = 1;     /* These magic numbers was provided by Les Kerr */
	id_packet.a22 = 755;
	id_packet.a23 = 65535;
