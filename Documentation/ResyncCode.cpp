						else if(_strcmpi(sResult, "RESYNC") == 0)
						{
							FILE *fTemp = NULL
							CStringBuilder SQL(10240);
							char sPKName[1024];
							char sTempFile[MAX_PATH];

							if(!GenerateTableCreationSQL(
								&lpCCI->cCustSQL, pSockSrvr, lpCCI->iClient,
								sDatabase, sSchema, sTable,
								sPKName, sizeof(sPKName), SQL.Buffer, SQL.Alloc))
							{
								WriteLog(pSockSrvr->icClientID[lpCCI->iClient],
									"Failed to generate table creation SQL for table re-sync.", EVENT_ERROR);

								SQL.Destroy();
								iResult = CMD_ERROR;
								break;
							}

							GetTempFileName(gsTempFilesPath, "SEC", 0, sTempFile);

							if(fopen_s(&fTemp, sTempFile, "wb") != 0)
							{
								WriteLog(pSockSrvr->icClientID[lpCCI->iClient],
									"Failed to create file for table re-sync.", EVENT_ERROR);

								SQL.Destroy();
								iResult = CMD_ERROR;
								break;
							}

							fwrite(SQL.Buffer, sizeof(char), strlen(SQL.Buffer), fTemp);
							SQL.Destroy();
							fclose(fTemp);

							CXMLChildBuilder MyWXML("TransactionData");
							MyWXML.Add("InitializeStep", iInitializeStep);
							MyWXML.Add("Database", sDatabase);
							MyWXML.Add("Schema", sSchema);
							MyWXML.Add("Table", sTable);
							SimpleTransfer(pSockSrvr, lpCCI->iClient, sTempFile, &MyWXML);
							MyWXML.Destroy();
						}
