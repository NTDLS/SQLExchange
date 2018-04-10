IF EXISTS (SELECT TOP 1 1 FROM Sys.Objects WHERE Type = 'P' AND Name = 'SaveSchemaSnapShot')
BEGIN
	DROP PROCEDURE [SaveSchemaSnapShot]
END
GO

CREATE PROCEDURE [SaveSchemaSnapShot]
(
	@DB VarChar(500)
)
AS
BEGIN
	DECLARE @SQL VarChar(8000)
	DECLARE @SCMTable VarChar(1000)
	DECLARE @ColTable VarChar(1000)

	SET @SCMTable = 'Schema_Objects_' + @DB
	SET @ColTable = 'Schema_Columns_' + @DB

	EXEC SQLExch_Replication..DropObjectIfExists 'SQLExch_Replication', @ColTable
	EXEC SQLExch_Replication..DropObjectIfExists 'SQLExch_Replication', @SCMTable

	IF @@VERSION LIKE '%SQL Server 2005%'
	BEGIN
		SET @SQL =
			'USE [' + @DB + ']; ' +
			'SELECT ' +
				'''' + @DB + ''' AS [DBName], ' +
				'SysC.* ' +
			'INTO ' +
				'SQLExch_Replication..[' + @ColTable + '] ' +
			'FROM ' +
				'Sys.Columns AS SysC ' +
			'INNER JOIN Sys.Objects AS SysO ' +
				'ON SysO.Object_ID = SysC.Object_ID ' +
			'WHERE ' +
				'SysO.[Type] = ''U'''
		EXEC(@SQL)

		SET @SQL =
			'USE [' + @DB + ']; ' +
			'SELECT ' +
				'''' + @DB + ''' AS [DBName], ' +
				'* ' +
			'INTO ' +
				'SQLExch_Replication..[' + @SCMTable + '] ' +
			'FROM ' +
				'Sys.Objects AS SysO ' +
			'WHERE ' +
				'SysO.[Type] = ''U'''
		EXEC(@SQL)
	END ELSE BEGIN
		SET @SQL =
			'USE [' + @DB + ']; ' +
			'SELECT ' +
				'''' + @DB + ''' AS [DBName], ' +
				'SysC.* ' +
			'INTO ' +
				'SQLExch_Replication..[' + @ColTable + '] ' +
			'FROM ' +
				'SysColumns AS SysC ' +
			'INNER JOIN SysObjects AS SysO ' +
				'ON SysO.ID = SysC.ID ' +
			'WHERE ' +
				'SysO.[xType] = ''U'''
		EXEC(@SQL)

		SET @SQL =
			'USE [' + @DB + ']; ' +
			'SELECT ' +
				'''' + @DB + ''' AS [DBName], ' +
				'* ' +
			'INTO ' +
				'SQLExch_Replication..[' + @SCMTable + '] ' +
			'FROM ' +
				'SysObjects AS SysO ' +
			'WHERE ' +
				'SysO.[xType] = ''U'''
		EXEC(@SQL)		
	END
END
GO

