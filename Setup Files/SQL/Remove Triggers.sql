/*
    This was designed to safely remove all existing SQL-Exchange
        related triggers from a SQL version 2005+ database.
*/
SET NOCOUNT ON;

DECLARE @SQL VarChar(8000)
DECLARE @Table VarChar(8000)
DECLARE @Trigger VarChar(8000)

DECLARE Cleanup CURSOR FAST_FORWARD FOR
	SELECT
		T.TABLE_NAME AS [TableName],
		'[' + T.TABLE_SCHEMA + '].[' + TR.name + ']' AS [TriggerName]
	 FROM
		INFORMATION_SCHEMA.TABLES AS T
	INNER JOIN sysobjects AS TR
		ON TR.xtype = 'TR'
		AND TR.parent_obj = OBJECT_ID(T.TABLE_CATALOG + '.' + T.TABLE_SCHEMA + '.' + T.TABLE_NAME)
		AND TR.name LIKE 'SQLExch[_]%%'
	 WHERE
		T.TABLE_TYPE = 'BASE TABLE'
	ORDER BY
		T.TABLE_NAME
OPEN Cleanup

FETCH FROM Cleanup INTO @Table, @Trigger

WHILE(@@FETCH_STATUS = 0)
BEGIN
	------------------------------------------------------------------------------------------

	IF @Trigger IS NOT NULL
	BEGIN--IF
		PRINT @Table + '.' + @Trigger
		EXEC('DROP TRIGGER ' + @Trigger)
	END--IF

	------------------------------------------------------------------------------------------
	FETCH FROM Cleanup INTO @Table, @Trigger
END

CLOSE Cleanup DEALLOCATE Cleanup
