ALTER PROCEDURE GetCompanyDatabaseInfo(@CompanyID INT) AS
BEGIN
	DECLARE @SQL		VarChar(MAX)
	DECLARE @DBName		VarChar(255)
	DECLARE @ID			INT
	DECLARE @Cluster	Decimal(16, 2)

	SELECT
		@Cluster = (1048576 / Val.[low])
	FROM
		Master..SPT_Values AS Val
	WHERE
		Val.Number = 1
		and Val.Type = 'E'

	IF EXISTS (SELECT TOP 1 1 FROM TempDB.Sys.Objects WHERE NAME = '##DBSize' AND TYPE = 'U')
	BEGIN
		DROP TABLE ##DBSize
	END

	CREATE TABLE ##DBSize
	(
		Data Decimal(16, 2),
		Logs Decimal(16, 2)
	)

		--TotalConnections	INT,
		--TotalBytesSent		Decimal(16, 2),
		--TotalBytesRcvd		Decimal(16, 2),
		--TotalTime			Decimal(16, 2)

	DECLARE @Results TABLE
	(
		ID					INT IDENTITY(1,1),
		DBPrefix			VarChar(255),
		DBName				VarChar(255),
		DBState				VarChar(255),
		DBPageVerifyOption	VarChar(255),
		DBRecoveryModel		VarChar(255),
		DataSize			Decimal(16, 2),
		LogsSize			Decimal(16, 2)
	)

	INSERT INTO @Results
	(
		DBName,
		DBPrefix,
		DBState,
		DBPageVerifyOption,
		DBRecoveryModel	
	)
	SELECT
		SysD.Name,
		C.CompDBPrefix,
		Replace(SysD.State_Desc, '_', ' '),
		Replace(SysD.Page_Verify_Option_Desc, '_', ' '),
		Replace(SysD.Recovery_Model_Desc, '_', ' ')
	FROM
		Companys AS C
	INNER JOIN Sys.Databases AS SysD
		ON SysD.Name LIKE 'SQLExch_' + C.CompDBPrefix + '%'
	WHERE
		C.ID = @CompanyID
	ORDER BY
		SysD.Name

	DECLARE DBs CURSOR FAST_FORWARD FOR
		SELECT
			ID,
			DBName
		FROM
			@Results
	OPEN DBs

	FETCH FROM DBs INTO @ID, @DBName

	WHILE(@@FETCH_STATUS = 0)
	BEGIN
		--------------------------------------------------------------------------------
		EXEC('TRUNCATE TABLE ##DBSize;')

		SET @SQL = 'INSERT INTO ##DBSize(Data, Logs)' +
			' SELECT ' +
			' (SELECT Sum(Convert(DEC(15),Size)) FROM [' + @DBName + ']..SysFiles WHERE (Status & 64 = 0)),' +
			' (SELECT Sum(Convert(DEC(15),Size)) FROM [' + @DBName + ']..SysFiles WHERE (Status & 64 <> 0))'

		EXEC(@SQL)

		UPDATE
			@Results
		SET
			DBName = Right(DBName, LEN(DBName) - LEN('SQLExch_' + DBPrefix + '_')),
			DataSize = (SELECT (Data / @Cluster) FROM ##DBSize),
			LogsSize = (SELECT (Logs / @Cluster) FROM ##DBSize)
		WHERE ID = @ID

		--------------------------------------------------------------------------------
		FETCH FROM DBs INTO @ID, @DBName
	END

	CLOSE DBs DEALLOCATE DBs

	SELECT
		R.DBName,
		R.DBState,
		R.DBPageVerifyOption,
		R.DBRecoveryModel,
		R.DataSize,
		R.LogsSize
	FROM
		@Results AS R
	ORDER BY
		R.DBName
END
GO

EXEC GetCompanyDatabaseInfo 10