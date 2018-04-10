IF EXISTS (SELECT TOP 1 1 FROM Sys.Objects WHERE Type = 'P' AND Name = 'CompareDatabaseSchemaToSnapshot')
BEGIN
	DROP PROCEDURE [CompareDatabaseSchemaToSnapshot]
END
GO

CREATE PROCEDURE [CompareDatabaseSchemaToSnapshot]
(
	@DB VarChar(500)
) AS
BEGIN
	DECLARE @SQL VarChar(8000)
	DECLARE @SCMTable VarChar(1000)
	DECLARE @ColTable VarChar(1000)

	SET @SCMTable = 'Schema_Objects_' + @DB
	SET @ColTable = 'Schema_Columns_' + @DB

	DECLARE @Changes TABLE
	(
		[Database]	VarChar(500),
		[Type]		VarChar(100),
		[Status]	VarChar(100),
		[Name]		VarChar(500)
	)

	SET @SQL =
		'USE [' + @DB + ']; ' +
		'SELECT ''' + @DB + ''' AS [Database], ''Table'' AS [Type], ''Missing'' AS [Status], OS.[Name] ' +
		'FROM [SQLExch_Replication]..[' + @SCMTable + '] AS OS ' +
		'WHERE OS.Name NOT IN (SELECT CS.Name FROM [' + @DB + '].Sys.Objects AS CS)' +
		'AND OS.Type = ''U'' ' +
		'ORDER BY CS.Name'
	INSERT INTO @Changes([Database], [Type], [Status], [Name]) EXEC(@SQL)

	SET @SQL =
		'USE [' + @DB + ']; ' +
		'SELECT ''' + @DB + ''' AS [Database], ''Table'' AS [Type], ''New'' AS [Status], CS.[Name] ' +
		'FROM [' + @DB + '].Sys.Objects AS CS ' +
		'WHERE CS.Name NOT IN (SELECT OS.Name FROM [SQLExch_Replication]..[' + @SCMTable + '] AS OS)' +
		'AND CS.Type = ''U'' ' +
		'ORDER BY CS.Name'
	INSERT INTO @Changes([Database], [Type], [Status], [Name]) EXEC(@SQL)

	SELECT * FROM @Changes


END
GO

