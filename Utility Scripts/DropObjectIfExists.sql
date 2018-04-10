IF EXISTS (SELECT TOP 1 1 FROM Sys.Objects WHERE Type = 'P' AND Name = 'DropObjectIfExists')
BEGIN
	DROP PROCEDURE [DropObjectIfExists]
END
GO

CREATE PROCEDURE [DropObjectIfExists]
(
	@DB VarChar(500),
	@ObjName VarChar(500)
) AS
BEGIN
	DECLARE @SQL VarChar(5000)

	IF @@VERSION LIKE '%SQL Server 2005%'
	BEGIN
		SET @SQL =
			'USE [' + @DB + ']; ' +
			'IF EXISTS ' +
			'(' +
			' SELECT TOP 1 1 FROM [' + @DB + '].Sys.Objects AS SysO' +
			' WHERE SysO.Type = ''u'' AND SysO.Name = ''' + @ObjName + '''' +
			') ' +
			'BEGIN DROP TABLE [' + @DB + ']..[' + @ObjName + '] END'
	END ELSE BEGIN
		SET @SQL =
			'USE [' + @DB + ']; ' +
			'IF EXISTS ' +
			'(' +
			' SELECT TOP 1 1 FROM [' + @DB + ']..SysObjects AS SysO' +
			' WHERE SysO.xType = ''u'' AND SysO.Name = ''' + @ObjName + '''' +
			') ' +
			'BEGIN DROP TABLE [' + @DB + ']..[' + @ObjName + '] END'
	END

	EXEC(@SQL)
END
GO
