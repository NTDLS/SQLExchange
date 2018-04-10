

IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AdminContacts]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[AdminContacts](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[FirstName] [varchar](200) NOT NULL,
	[LastName] [varchar](200) NOT NULL,
	[Address1] [varchar](200) NULL,
	[Address2] [varchar](200) NULL,
	[City] [varchar](200) NULL,
	[State] [varchar](200) NULL,
	[PostalCode] [varchar](200) NULL,
	[Region] [varchar](200) NULL,
	[Country] [varchar](200) NULL,
	[EMailAddress] [varchar](200) NULL,
	[HomePhone] [varchar](200) NULL,
	[MobilePhone] [varchar](200) NULL,
	[Fax] [varchar](200) NULL,
	[OfficePhone] [varchar](200) NULL,
	[OfficeExt] [varchar](200) NULL,
	[Active] [bit] NOT NULL,
	[CreatedDate] [datetime] NOT NULL,
	[Desription] [varchar](max) NULL
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
END

GO
SET ANSI_PADDING ON

IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[AdminContacts]') AND name = N'IX_AdminContacts')
CREATE UNIQUE NONCLUSTERED INDEX [IX_AdminContacts] ON [dbo].[AdminContacts]
(
	[FirstName] ASC,
	[LastName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[AdminContacts]') AND name = N'PK_AdminContacts')
ALTER TABLE [dbo].[AdminContacts] ADD  CONSTRAINT [PK_AdminContacts] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[ClientVersions]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[ClientVersions](
	[Version] [varchar](25) NOT NULL,
	[Status] [varchar](25) NOT NULL,
	[UpdateURL] [varchar](1024) NULL,
	[CreatedDate] [datetime] NULL CONSTRAINT [DF_ClientVersions_Created]  DEFAULT (getdate())
) ON [PRIMARY]
END

GO
SET ANSI_PADDING ON

IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[ClientVersions]') AND name = N'PK_ClientVersions_1')
ALTER TABLE [dbo].[ClientVersions] ADD  CONSTRAINT [PK_ClientVersions_1] PRIMARY KEY CLUSTERED 
(
	[Version] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Company]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[Company](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[Name] [varchar](100) NOT NULL,
	[Password] [varchar](64) NOT NULL,
	[DatabasePrefix] [varchar](50) NOT NULL CONSTRAINT [DF_Companys_CompDBPrefix]  DEFAULT ('Not_Configured'),
	[InitializeStep] [int] NULL CONSTRAINT [DF_Companys_InitStep]  DEFAULT ((0)),
	[MonitorWaitTime] [bit] NOT NULL CONSTRAINT [DF_Companys_MonitorWaitTime]  DEFAULT ((1)),
	[AverageWaitTime] [decimal](18, 10) NULL,
	[LastConnectDate] [datetime] NULL,
	[LastKnownIPAddress] [varchar](20) NULL,
	[OfflineProcessingDatabase] [varchar](255) NULL,
	[OfflineProcessingSchema] [varchar](255) NULL,
	[OfflineProcessingTable] [varchar](255) NULL,
	[PricingDiscount] [decimal](18, 3) NOT NULL CONSTRAINT [DF_Companys_Discount]  DEFAULT ((0)),
	[APIAuthenticationString] [uniqueidentifier] NOT NULL CONSTRAINT [DF_Companys_APIAuthString]  DEFAULT (newid()),
	[CreatedDate] [datetime] NULL CONSTRAINT [DF_Companys_Created]  DEFAULT (getdate()),
	[Comment] [varchar](1000) NULL,
	[Active] [bit] NOT NULL CONSTRAINT [DF_Companys_Active]  DEFAULT ((1))
) ON [PRIMARY]
END

GO
SET ANSI_PADDING ON

IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Company]') AND name = N'IX_Company')
CREATE UNIQUE NONCLUSTERED INDEX [IX_Company] ON [dbo].[Company]
(
	[Name] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[Company]') AND name = N'PK_Company')
ALTER TABLE [dbo].[Company] ADD  CONSTRAINT [PK_Company] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[PricingScheme]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[PricingScheme](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[AsOfDate] [datetime] NOT NULL,
	[PerConnect] [money] NULL,
	[PerMinute] [money] NULL,
	[PerMBTrans] [money] NULL,
	[PerGBStore] [money] NULL,
	[PerDB] [money] NULL,
	[PerKernelCPUTime] [money] NULL,
	[PerUserCPUTime] [money] NULL,
	[PerCPUTime] [money] NULL,
	[Active] [bit] NOT NULL,
	[Comment] [varchar](1000) NULL
) ON [PRIMARY]
END

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[PricingScheme]') AND name = N'PK_PricingScheme')
ALTER TABLE [dbo].[PricingScheme] ADD  CONSTRAINT [PK_PricingScheme] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CompanyAdminContacts]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CompanyAdminContacts](
	[AdminContactID] [int] NOT NULL,
	[CompanyID] [int] NOT NULL
) ON [PRIMARY]
END

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanyAdminContacts]') AND name = N'PK_CompanyAdminContacts_1')
ALTER TABLE [dbo].[CompanyAdminContacts] ADD  CONSTRAINT [PK_CompanyAdminContacts_1] PRIMARY KEY CLUSTERED 
(
	[AdminContactID] ASC,
	[CompanyID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CompanyDistributionPoints]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CompanyDistributionPoints](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[CompanyID] [int] NOT NULL,
	[SQL] [varchar](max) NOT NULL,
	[CreatedDate] [datetime] NOT NULL
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
END

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanyDistributionPoints]') AND name = N'PK_CompanyDistributionPoints')
ALTER TABLE [dbo].[CompanyDistributionPoints] ADD  CONSTRAINT [PK_CompanyDistributionPoints] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CompanySchedules]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CompanySchedules](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[CompanyID] [int] NOT NULL,
	[Sunday] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Sunday]  DEFAULT ((1)),
	[Monday] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Monday]  DEFAULT ((1)),
	[Tuesday] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Tuesday]  DEFAULT ((1)),
	[Wednesday] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Wednesday]  DEFAULT ((1)),
	[Thursday] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Thursday]  DEFAULT ((1)),
	[Friday] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Friday]  DEFAULT ((1)),
	[Saturday] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Saturday]  DEFAULT ((1)),
	[StartTime] [varchar](50) NOT NULL CONSTRAINT [DF_CompanySchedules_StartTime]  DEFAULT ('9:00'),
	[EndTime] [varchar](50) NOT NULL CONSTRAINT [DF_CompanySchedules_EndTime]  DEFAULT ('17:00'),
	[Increment] [int] NOT NULL CONSTRAINT [DF_CompanySchedules_Increment]  DEFAULT ((15)),
	[Enforce] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Enforce]  DEFAULT ((1)),
	[AsOfDate] [datetime] NOT NULL CONSTRAINT [DF_CompanySchedules_AsOfDate]  DEFAULT (getdate()),
	[Active] [bit] NOT NULL CONSTRAINT [DF_CompanySchedules_Active]  DEFAULT ((1))
) ON [PRIMARY]
END

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanySchedules]') AND name = N'IX_CompanySchedules')
CREATE UNIQUE NONCLUSTERED INDEX [IX_CompanySchedules] ON [dbo].[CompanySchedules]
(
	[CompanyID] ASC,
	[AsOfDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanySchedules]') AND name = N'PK_CompanySchedules')
ALTER TABLE [dbo].[CompanySchedules] ADD  CONSTRAINT [PK_CompanySchedules] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CompanyStatistics]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CompanyStatistics](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[CompanyID] [int] NOT NULL,
	[ConnectTime] [datetime] NULL,
	[DisconnectTime] [datetime] NULL,
	[BytesSent] [int] NULL,
	[BytesReceived] [int] NULL,
	[KernelTime] [decimal](32, 16) NULL,
	[UserTime] [decimal](32, 16) NULL,
	[Paid] [bit] NULL,
	[WasInitialize] [bit] NOT NULL,
	[CreatedDate] [datetime] NOT NULL CONSTRAINT [DF_CompanyStatistics_CreatedDate]  DEFAULT (getdate())
) ON [PRIMARY]
END

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanyStatistics]') AND name = N'IX_CompanyStats_Company_Key')
CREATE NONCLUSTERED INDEX [IX_CompanyStats_Company_Key] ON [dbo].[CompanyStatistics]
(
	[CompanyID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanyStatistics]') AND name = N'PK_CompanyStatistics')
ALTER TABLE [dbo].[CompanyStatistics] ADD  CONSTRAINT [PK_CompanyStatistics] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CompanyTableRetrieval]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CompanyTableRetrieval](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[CompanyID] [int] NOT NULL,
	[DatabaseName] [varchar](512) NOT NULL,
	[OwnerName] [varchar](512) NOT NULL,
	[TableName] [varchar](512) NOT NULL,
	[CreatedDate] [datetime] NOT NULL CONSTRAINT [DF_CompanyTableRetrieval_CreatedDate]  DEFAULT (getdate()),
	[Complete] [bit] NOT NULL
) ON [PRIMARY]
END

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanyTableRetrieval]') AND name = N'PK_CompanyTableRetrieval')
ALTER TABLE [dbo].[CompanyTableRetrieval] ADD  CONSTRAINT [PK_CompanyTableRetrieval] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CompanyUsers]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CompanyUsers](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[CompanyID] [int] NOT NULL,
	[FirstName] [varchar](50) NOT NULL,
	[LastName] [varchar](50) NOT NULL,
	[UserName] [varchar](50) NOT NULL,
	[Password] [varchar](50) NOT NULL,
	[SecurityLevel] [smallint] NULL,
	[Created] [datetime] NOT NULL,
	[Active] [bit] NOT NULL,
	[Comment] [varchar](1000) NULL
) ON [PRIMARY]
END

GO
SET ANSI_PADDING ON

IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanyUsers]') AND name = N'IX_CompanyUsers')
CREATE UNIQUE NONCLUSTERED INDEX [IX_CompanyUsers] ON [dbo].[CompanyUsers]
(
	[CompanyID] ASC,
	[UserName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CompanyUsers]') AND name = N'PK_CompanyUsers')
ALTER TABLE [dbo].[CompanyUsers] ADD  CONSTRAINT [PK_CompanyUsers] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]

GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyAdminContacts_AdminContacts]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyAdminContacts]'))
ALTER TABLE [dbo].[CompanyAdminContacts]  WITH CHECK ADD  CONSTRAINT [FK_CompanyAdminContacts_AdminContacts] FOREIGN KEY([AdminContactID])
REFERENCES [dbo].[AdminContacts] ([ID])
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyAdminContacts_AdminContacts]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyAdminContacts]'))
ALTER TABLE [dbo].[CompanyAdminContacts] CHECK CONSTRAINT [FK_CompanyAdminContacts_AdminContacts]

GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyAdminContacts_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyAdminContacts]'))
ALTER TABLE [dbo].[CompanyAdminContacts]  WITH CHECK ADD  CONSTRAINT [FK_CompanyAdminContacts_Company] FOREIGN KEY([CompanyID])
REFERENCES [dbo].[Company] ([ID])
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyAdminContacts_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyAdminContacts]'))
ALTER TABLE [dbo].[CompanyAdminContacts] CHECK CONSTRAINT [FK_CompanyAdminContacts_Company]

GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyDistributionPoints_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyDistributionPoints]'))
ALTER TABLE [dbo].[CompanyDistributionPoints]  WITH CHECK ADD  CONSTRAINT [FK_CompanyDistributionPoints_Company] FOREIGN KEY([CompanyID])
REFERENCES [dbo].[Company] ([ID])
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyDistributionPoints_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyDistributionPoints]'))
ALTER TABLE [dbo].[CompanyDistributionPoints] CHECK CONSTRAINT [FK_CompanyDistributionPoints_Company]

GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanySchedules_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanySchedules]'))
ALTER TABLE [dbo].[CompanySchedules]  WITH CHECK ADD  CONSTRAINT [FK_CompanySchedules_Company] FOREIGN KEY([CompanyID])
REFERENCES [dbo].[Company] ([ID])
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanySchedules_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanySchedules]'))
ALTER TABLE [dbo].[CompanySchedules] CHECK CONSTRAINT [FK_CompanySchedules_Company]

GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyStatistics_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyStatistics]'))
ALTER TABLE [dbo].[CompanyStatistics]  WITH CHECK ADD  CONSTRAINT [FK_CompanyStatistics_Company] FOREIGN KEY([CompanyID])
REFERENCES [dbo].[Company] ([ID])
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyStatistics_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyStatistics]'))
ALTER TABLE [dbo].[CompanyStatistics] CHECK CONSTRAINT [FK_CompanyStatistics_Company]

GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyTableRetrieval_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyTableRetrieval]'))
ALTER TABLE [dbo].[CompanyTableRetrieval]  WITH CHECK ADD  CONSTRAINT [FK_CompanyTableRetrieval_Company] FOREIGN KEY([CompanyID])
REFERENCES [dbo].[Company] ([ID])
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyTableRetrieval_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyTableRetrieval]'))
ALTER TABLE [dbo].[CompanyTableRetrieval] CHECK CONSTRAINT [FK_CompanyTableRetrieval_Company]

GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyUsers_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyUsers]'))
ALTER TABLE [dbo].[CompanyUsers]  WITH CHECK ADD  CONSTRAINT [FK_CompanyUsers_Company] FOREIGN KEY([CompanyID])
REFERENCES [dbo].[Company] ([ID])
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_CompanyUsers_Company]') AND parent_object_id = OBJECT_ID(N'[dbo].[CompanyUsers]'))
ALTER TABLE [dbo].[CompanyUsers] CHECK CONSTRAINT [FK_CompanyUsers_Company]

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxUpdateCompanyWaitTimes]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxUpdateCompanyWaitTimes]
AS
BEGIN
	SET NOCOUNT ON
	DECLARE @ConnectTime		DateTime
	DECLARE @LastConnectTime	DateTime
	DECLARE @DisconnectTime		DateTime
	DECLARE @WaitTime			Decimal(18, 10)
	DECLARE @Average			Decimal(18, 10)
	DECLARE @CompanyID			Int
	DECLARE @CompStats TABLE
	(
		[WaitTime] Decimal(18, 10)
	)
	DECLARE Companys CURSOR FAST_FORWARD FOR
		SELECT
			ID
		FROM
			Companys
		WHERE
			Active = 1
	OPEN Companys
	FETCH NEXT FROM Companys INTO @CompanyID
	WHILE(@@FETCH_STATUS = 0)
	BEGIN
		------------------------------------------------------------------------------------------------------
		DECLARE Stats CURSOR FAST_FORWARD FOR
			SELECT TOP 1000
				CS.ConnectTime,
				CS.DisconnectTime
			FROM
				CompanyStats AS CS
			WHERE
				CS.Company_Key = @CompanyID		
				AND IsNull(CS.Init, 0) = 0		
			ORDER BY
				CS.ID ASC
		OPEN Stats
		FETCH NEXT FROM Stats INTO @ConnectTime, @DisconnectTime
		WHILE(@@FETCH_STATUS = 0)
		BEGIN
			------------------------------------------------------------------------------------------------------
			IF NOT @LastConnectTime IS NULL
			BEGIN
				INSERT INTO @CompStats ([WaitTime]) SELECT DateDiff(n, @LastConnectTime, @ConnectTime)
			END
			SET @LastConnectTime = @DisconnectTime
			------------------------------------------------------------------------------------------------------
			FETCH NEXT FROM Stats INTO @ConnectTime, @DisconnectTime
		END
		CLOSE Stats DEALLOCATE Stats
		UPDATE
			Companys
		SET
			AverageWaitTime = (SELECT Avg(WaitTime) FROM @CompStats)
		WHERE
			ID = @CompanyID
		DELETE FROM @CompStats
		SET @LastConnectTime = NULL
		------------------------------------------------------------------------------------------------------
		FETCH NEXT FROM Companys INTO @CompanyID
	END
	CLOSE Companys DEALLOCATE Companys
END

' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetAllCompanyNames]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE GetAllCompanyNames AS
BEGIN
	SELECT
		C.[Name]
	FROM
		Company AS C
	ORDER BY
		C.Name
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetBasicCompanyInfoByName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE GetBasicCompanyInfoByName
(
	@CompanyName VarChar(100)
) AS
BEGIN
	SELECT
		C.ID,
		C.[Password],
		C.[InitializeStep],
		C.[DatabasePrefix],
		C.[Active]
	FROM
		Company AS C
	WHERE
		C.Name = @CompanyName
END' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetClientVersionInfo]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE GetClientVersionInfo
(
	@Version VarChar(100)
) AS
BEGIN
	SELECT
		CV.[Status],
		CV.[UpdateURL]
	FROM
		ClientVersions AS CV
	WHERE
		CV.Version = @Version
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetCompanyDatabasePrefixByName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE GetCompanyDatabasePrefixByName
(
	@CompanyName VarChar(100)
) AS
BEGIN
	SELECT
		C.[DatabasePrefix]
	FROM
		Company AS C
	WHERE
		C.Name = @CompanyName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetCompanyScheduleByName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[GetCompanyScheduleByName]
(
	@CompanyName VarChar(100)
) AS
BEGIN
	SELECT
		CS.Sunday,
		CS.Monday,
		CS.Tuesday,
		CS.Wednesday,
		CS.Thursday,
		CS.Friday,
		CS.Saturday,
		CS.StartTime,
		CS.EndTime,
		CS.Increment,
		CS.Enforce
	FROM
		CompanySchedules AS CS
	INNER JOIN Company AS C
		ON C.ID = CS.CompanyID
	WHERE
		C.Name = @CompanyName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GetOfflineProcessingTableByCompanyName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE GetOfflineProcessingTableByCompanyName
(
	@CompanyName VarChar(100)
) AS
BEGIN
	SELECT
		IsNull(C.OfflineProcessingTable, '''') AS TableName
	FROM
		Company AS C
	WHERE
		C.Name = @CompanyName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[InsertCompanyStatistics]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].InsertCompanyStatistics
(
	@CompanyName VarChar(100),
	@ConnectTime DateTime,
	@DisconnectTime DateTime,
	@BytesSent Int,
	@BytesReceived Int,
	@KernelTime Decimal(32, 16),
	@UserTime Decimal(32, 16),
	@WasInitialize Bit
) AS
BEGIN
	INSERT INTO CompanyStatistics
	(
		CompanyID,
		ConnectTime,
		DisconnectTime,
		BytesSent,
		BytesReceived,
		KernelTime,
		UserTime,
		WasInitialize,
		CreatedDate
	)
	SELECT
		C.ID AS CompanyID,
		@ConnectTime AS ConnectTime,
		@DisconnectTime AS DisconnectTime,
		@BytesSent AS BytesSent,
		@BytesReceived AS BytesReceived,
		@KernelTime AS KernelTime,
		@UserTime AS UserTime,
		@WasInitialize AS WasInitialize,
		GetDate() AS CreatedDate
	FROM
		Company AS C
	WHERE
		C.Name = @CompanyName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[InsertCompanyTableRetrieval]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE InsertCompanyTableRetrieval
(
	@CompanyName VarChar(100),
	@DatabaseName VarChar(255),
	@OwnerName VarChar(255),
	@TableName VarChar(255)
) AS
BEGIN
	INSERT INTO CompanyTableRetrieval
	(
		CompanyID,
		DatabaseName,
		OwnerName,
		TableName,
		CreatedDate,
		Complete
	)
	SELECT
		C.ID AS CompanyID,
		@DatabaseName AS DatabaseName,
		@OwnerName AS OwnerName,
		@TableName AS TableName,
		GetDate() AS CreatedDate,
		0 AS Complete
	FROM
		Company AS C
	WHERE
		C.Name = @CompanyName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SetBasicCompanyInfoByName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[SetBasicCompanyInfoByName]
(
	@CompanyName VarChar(100),
	@IPAddress VarChar(25) = NULL,
	@InitializeStep INT = NULL
) AS
BEGIN
	UPDATE
		Company
	SET
		LastKnownIPAddress = IsNull(@IPAddress, LastKnownIPAddress),
		InitializeStep = IsNull(@InitializeStep, InitializeStep),
		LastConnectDate = GetDate()
	WHERE
		[Name] = @CompanyName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SetCompanyInitStepByName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE SetCompanyInitStepByName
(
	@CompanyName VarChar(100),
	@InitializeStep INT
) AS
BEGIN
	UPDATE
		Company
	SET
		InitializeStep = @InitializeStep
	WHERE
		[Name] = @CompanyName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SetCompanyTableRetrieval]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE SetCompanyTableRetrieval
(
	@CompanyName VarChar(100),
	@DatabaseName VarChar(255),
	@OwnerName VarChar(255),
	@TableName VarChar(255),
	@Complete BIT
) AS
BEGIN
	UPDATE
		CompanyTableRetrieval
	 SET
		Complete = @Complete
	 WHERE
		DatabaseName = @DatabaseName
		AND OwnerName = @OwnerName
		AND TableName = @TableName
		AND CompanyID = 
			(
				SELECT
					C.ID
				FROM
					Company AS C
				WHERE
					C.Name = @CompanyName
			)
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SetOfflineProcessingByCompanyName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[SetOfflineProcessingByCompanyName]
(
	@CompanyName VarChar(100),
	@DatabaseName VarChar(255),
	@SchemaName VarChar(255),
	@TableName VarChar(255)
) AS
BEGIN
	UPDATE
		Company
	SET
		OfflineProcessingDatabase = @DatabaseName,
		OfflineProcessingSchema = @SchemaName,
		OfflineProcessingTable = @TableName
	WHERE
		[Name] = @CompanyName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxGetAPIDatabaseName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxGetAPIDatabaseName]
(
	@AuthString UniqueIdentifier,
	@Database Varchar(255)
) AS
BEGIN
	SELECT
		SysDB.Name AS DB
	FROM
		Companys AS C
	INNER JOIN Sys.Databases AS SysDB
		ON SysDB.Name = (''SQLExch_'' + C.CompDBPrefix + ''_'' + @Database)
	WHERE
		C.Active = 1
		AND C.APIAuthString = @AuthString
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxGetAPIPKColumns]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxGetAPIPKColumns]
(
	@Database VarChar(255),
	@Table VarChar(255)
) AS
BEGIN
	DECLARE @SQL VarChar(MAX)
	SET @SQL = 
	''USE ['' + @Database + '']; '' +
	''SELECT '' +
		''SysC.Name AS [Column], '' +
		''Upper(SysT.Name) AS [Type], '' +
		''SysC.Max_Length AS [Length] '' +
	''FROM '' +
		''sys.indexes AS SysI '' +
	''INNER JOIN Sys.Index_Columns AS SysIC '' +
		''ON SysIC.Object_ID = SysI.Object_ID '' +
		''AND SysIC.Index_ID = SysI.Index_ID '' +
	''INNER JOIN Sys.Columns AS SysC '' +
		''ON SysC.Object_ID = SysI.Object_ID '' +
		''AND SysC.Column_ID = SysIC.Column_ID '' +
	''INNER JOIN Sys.Types AS SysT '' +
		''ON SysT.User_Type_ID = SysC.User_Type_ID '' +
	''WHERE '' +
		''SysI.Is_Primary_Key = 1 '' +
		''AND SysI.Object_ID = Object_ID('''''' + @Table + '''''')''
	EXEC(@SQL)
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxGetClientHitAvarages]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxGetClientHitAvarages]
AS
BEGIN
	SELECT
		Count(Hits) AS Days,
		Max(Hits) AS MaxHits,
		Min(Hits) AS MinHits,
		Min(Date) AS MinDate,
		Max(Date) AS MaxDate,
		Avg(Hits) AS AverageHits
	FROM
		GraphHits AS GH
	WHERE
		GH.[Date] >= (SELECT Convert(DateTime, Convert(VarChar, Min(ConnectTime), 101)) FROM CompanyStats)
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxGetCompanyDatabaseInfo]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxGetCompanyDatabaseInfo](@CompanyID INT) AS
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
		and Val.Type = ''E''
	IF EXISTS (SELECT TOP 1 1 FROM TempDB.Sys.Objects WHERE NAME = ''##DBSize'' AND TYPE = ''U'')
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
		Replace(SysD.State_Desc, ''_'', '' ''),
		Replace(SysD.Page_Verify_Option_Desc, ''_'', '' ''),
		Replace(SysD.Recovery_Model_Desc, ''_'', '' '')
	FROM
		Companys AS C
	INNER JOIN Sys.Databases AS SysD
		ON SysD.Name LIKE ''SQLExch_'' + C.CompDBPrefix + ''%''
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
		EXEC(''TRUNCATE TABLE ##DBSize;'')
		SET @SQL = ''INSERT INTO ##DBSize(Data, Logs)'' +
			'' SELECT '' +
			'' (SELECT Sum(Convert(DEC(15),Size)) FROM ['' + @DBName + '']..SysFiles WHERE (Status & 64 = 0)),'' +
			'' (SELECT Sum(Convert(DEC(15),Size)) FROM ['' + @DBName + '']..SysFiles WHERE (Status & 64 <> 0))''
		EXEC(@SQL)
		UPDATE
			@Results
		SET
			DBName = Right(DBName, LEN(DBName) - LEN(''SQLExch_'' + DBPrefix + ''_'')),
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
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxGetCompanyStats]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxGetCompanyStats]
AS
BEGIN
	SELECT
		C.CompName,
		CS.Company_Key,
		Sum(DateDiff(s, CS.ConnectTime, CS.DisconnectTime)) AS SecondsConnected,
		Sum(IsNull(CS.BytesSent, 0)) AS BytesSent,
		Sum(IsNull(CS.BytesReceived, 0)) AS BytesRecv,
		Convert(Decimal(16, 2), Sum(IsNull(CS.KernelTime, 0))) AS KernelTime,
		Convert(Decimal(16, 2), Sum(IsNull(CS.UserTime, 0))) AS UserTime
	FROM
		CompanyStats AS CS
	INNER JOIN Companys AS C
		ON C.ID = CS.Company_Key
	GROUP BY
		CS.Company_Key,
		C.CompName
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxGetCustomerWarningLevel]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxGetCustomerWarningLevel]
AS
BEGIN
	DECLARE @SQL VarChar(255)
	SELECT @SQL = ''SELECT'' +
		'' Company_Key,'' +
		'' IsNull(['' + DateName(dw, GetDate()) + ''], 0) AS RunsToday'' +
		'' INTO ##RunToday '' +
		'' FROM CompanySchedules'' +
		'' WHERE Active = 1''
	EXEC(@SQL)
	SELECT
		Warnings.Companys_ID,
		Warnings.WarningLevel
	FROM
	(
		SELECT
			Companys.ID AS Companys_ID,
			CASE
				WHEN DateDiff(n, LastConnect, GetDate()) > AverageWaitTime * 1.50 THEN 5
				WHEN DateDiff(n, LastConnect, GetDate()) > AverageWaitTime * 1.40 THEN 4
				WHEN DateDiff(n, LastConnect, GetDate()) > AverageWaitTime * 1.30 THEN 3
				WHEN DateDiff(n, LastConnect, GetDate()) > AverageWaitTime * 1.20 THEN 2
				WHEN DateDiff(n, LastConnect, GetDate()) > AverageWaitTime * 1.10 THEN 1
				ELSE 0
			END AS WarningLevel
		FROM
			Companys AS Companys
		INNER JOIN CompanySchedules AS CS
			ON CS.Company_Key = Companys.ID
		INNER JOIN ##RunToday AS RT
			ON RT.Company_Key = Companys.ID
		WHERE
			NOT LastConnect IS NULL			-- Has connected in the past.
			AND NOT AverageWaitTime IS NULL -- Average wait time has been calculated.
			AND CS.Active = 1				-- Has an active schedule.
			AND Companys.Active = 1			-- The company record is set to active.
			AND InitStep = -1				-- Is not currently initializing.
			AND MonitorWaitTime = 1			-- Company set to be monitored.
			AND RT.RunsToday = 1			-- Schedule is set to run today.
			AND CS.Enforce = 1				-- Schedule is enforced.
			AND								-- Within the time window with a tolerance of "Increment" minutes.
				(Convert(DateTime, Convert(VarChar, GetDate(), 108)) >= DateAdd(mi, Increment, Convert(DateTime, CS.StartTime))		
				AND Convert(DateTime, Convert(VarChar, GetDate(), 108)) <= DateAdd(mi, 0-Increment, Convert(DateTime, CS.EndTime)))
	) AS Warnings
	WHERE
		Warnings.WarningLevel > 0
	DROP TABLE ##RunToday
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxProcessCompanyAlerts]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxProcessCompanyAlerts]
AS
BEGIN
	DECLARE @EmailAddress	VarChar(200)
	DECLARE @CompanyName	VarChar(200)
	DECLARE @LastConnect	DateTime
	DECLARE @CompanyID		Int
	DECLARE @WarningLevel	Int
	DECLARE @Version		VarChar(16)
	DECLARE @TableHTML		VarChar(MAX)
	EXEC [UpdateCompanyWaitTimes]
	SELECT TOP 1
		@Version = Version
	FROM
		ClientVersions
	WHERE
		Status = ''Current''
	DECLARE @CWL TABLE
	(
		ID Int,
		WarnLevel Int
	)
	INSERT INTO @CWL EXEC GetCustomerWarningLevel
	DECLARE Alerts Cursor FAST_FORWARD FOR
		SELECT
			Companys.ID,
			Companys.CompName,
			Companys.LastConnect,
			CWL.WarnLevel
		FROM
			Companys AS Companys
		INNER JOIN @CWL AS CWL
			ON CWL.ID = Companys.ID
		WHERE
			CWL.WarnLevel > 0
	OPEN Alerts
	FETCH NEXT FROM Alerts INTO @CompanyID, @CompanyName, @LastConnect, @WarningLevel
	WHILE(@@FETCH_STATUS = 0)
	BEGIN
		IF @WarningLevel > 0
		BEGIN
			DECLARE EmailAddresses Cursor FAST_FORWARD FOR
				SELECT
					AC.EmailAddress
				FROM
					AdminContacts AS AC
				INNER JOIN CompanyAdminContacts AS CAC
					ON CAC.AdminContact_Key = AC.ID
				WHERE
					CAC.Company_Key = @CompanyID
					AND AC.Active = 1
			OPEN EmailAddresses
			FETCH NEXT FROM EmailAddresses INTO @EmailAddress
			WHILE(@@FETCH_STATUS = 0)
			BEGIN
				SELECT @CompanyName, @WarningLevel, @EmailAddress
				SET @TableHTML =
					''<H1>SQL-Exchange Alert!</H1>'' +
					''<table border="1">'' +
					''  <tr>'' +
					''    <td><B>Company</B></td>'' +
					''    <td>'' + @CompanyName + ''</td>'' +
					''  </tr>'' +
					''  <tr>'' +
					''    <td><B>Warning Level</B></td>'' +
					''    <td>'' + Convert(Varchar, @WarningLevel) + ''</td>'' +
					''  </tr>'' +
					''  <tr>'' +
					''    <td><B>Last Connect</B></td>'' +
					''    <td>'' + Convert(VarChar, @LastConnect) + ''</td>'' +
					''  </tr>'' +
					''</table><BR><BR>'' +
					''Email sent '' + Convert(VarChar, GetDate()) + ''<BR><BR>'' +
					''<I>NetworkDLS SQL-Exchange Version '' + @Version + ''</I>'' +
					''<BR><BR>'' +
					''<font size=1 face="Courier New">Important: This e-mail is intended for the use of the addressee and may contain information that is confidential, commercially valuable or subject to legal or parliamentary privilege. If you are not the intended recipient you are notified that any use or dissemination of this communication is strictly prohibited. If you have received this communication in error please notify the sender immediately. Furthermore, NetworkDLS monitors its incoming and outgoing emails, however, we do not guarantee that this email or the attachment/s are unaffected by computer virus, corruption or other defects.</font>''
				EXEC msdb.dbo.sp_Send_DBMail
					@Profile_Name = ''Primary DB Mail'',
					@Recipients = @EmailAddress,
					@Body = @TableHTML,
					@Body_Format = ''HTML'',
					@Subject = ''Automated Alert''
				FETCH NEXT FROM EmailAddresses INTO @EmailAddress
			END
			CLOSE EmailAddresses DEALLOCATE EmailAddresses
		END
		FETCH NEXT FROM Alerts INTO @CompanyID, @CompanyName, @LastConnect, @WarningLevel
	END
	CLOSE Alerts DEALLOCATE Alerts
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxReCalcClientHits]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxReCalcClientHits]
AS
BEGIN
	-------------------------------------------------------------------------
	--This stored procedure will delete all records from the [GraphHits]
	--	table that are more than one year old and recalculate todays
	--	and any missing days in the table.
	-------------------------------------------------------------------------
	SET NOCOUNT ON;
	--------------------------------------------------------------------------------------------
	IF NOT Exists (SELECT * FROM SQLExch_Index..SysObjects WHERE Type = ''U'' AND Name = ''GraphHits'')
	BEGIN
		SELECT 0 AS Hits, Convert(DateTime, GetDate()) AS Date INTO SQLExch_Index..GraphHits
		TRUNCATE TABLE SQLExch_Index..GraphHits
		--DROP TABLE SQLExch_Index..GraphHits
	END;
	--Variable declaration block.
	DECLARE @CurDay	DateTime
	DECLARE @Today	DateTime
	DECLARE @Hits	Int
	SET @Today	=	(SELECT Convert(DateTime, Convert(VarChar, GetDate(), 101)))
	SET @CurDay	=	(SELECT MAX(Date) FROM SQLExch_Index..GraphHits)
	IF @CurDay IS NULL
	BEGIN
		SET @CurDay = (SELECT DateAdd(YY, -1, Convert(VarChar, GetDate(), 101)))
	END
	--------------------------------------------------------------------------------------------
	DELETE FROM
		SQLExch_Index..GraphHits
	WHERE
		Convert(VarChar, [Date], 101) = @CurDay
	--------------------------------------------------------------------------------------------
	DELETE FROM
		SQLExch_Index..GraphHits
	WHERE
		Date < DateAdd(dd, -364, Convert(VarChar, GetDate(), 101))
	--------------------------------------------------------------------------------------------
	WHILE (@CurDay <= @Today)
	BEGIN
		SET @Hits = 
			IsNull((
				SELECT
					Count(0)
				FROM
					CompanyStats
				WHERE
					Convert(VarChar, ConnectTime, 101) = @CurDay
			), 0)

		INSERT INTO SQLExch_Index..GraphHits
		(
			Hits,
			Date
		)
		SELECT
			@Hits,	--[Hits]
			@CurDay	--[Date]
		PRINT
			Convert(VarChar, GetDate(), 101) + '' '' +
			Convert(VarChar, GetDate(), 108) +
			'' (Day: '' + Convert(VarChar, @CurDay, 101) + '', '' +
			'' Hits: '' + Convert(VarChar(10), @Hits) + '')''
		SET @CurDay = DateAdd(dd, 1, @CurDay)
	END
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxSkewConnectTimes]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxSkewConnectTimes]
(
	@StartTime AS DateTime,
	@EndTime AS DateTime
)
AS
BEGIN
	UPDATE
		CompanySchedules
	SET
		StartTime = Left(Convert(VarChar, DateAdd(N, (ID % 10), Convert(DateTime, @StartTime)), 108), 5),
		EndTime = Left(Convert(VarChar, DateAdd(N, (ID % 10), Convert(DateTime, @EndTime)), 108), 5)
	WHERE
		Enforce = 1
		AND Active = 1
END
' 
END

GO


IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[xxxWebGetCompanyUsers]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[xxxWebGetCompanyUsers]
(
	@CompanyID INT
)
AS
BEGIN
	SET NOCOUNT ON;
	SELECT
		CU.ID,
		CU.FirstName,
		CU.LastName,
		CU.Username,
		CU.Password,
		CU.SecurityLevel,
		CU.Active,
		CU.Comment
	FROM
		CompanyUsers AS CU
	WHERE
		CU.Company_Key = @CompanyID
END
' 
END

GO
ALTER TABLE [dbo].[ClientVersions] NOCHECK CONSTRAINT ALL
GO
IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '0.0.0.0') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('0.0.0.0','AutoUpdate','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'AutoUpdate',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '0.0.0.0'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.0') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.0','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.0'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.1') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.1','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.1'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.2') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.2','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.2'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.3') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.3','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.3'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.4') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.4','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.4'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.5') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.5','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.5'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.6') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.6','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.6'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.7') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.7','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.7'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.8') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.8','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.8'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.0.9') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.0.9','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.0.9'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.0') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.0','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.0'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.1') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.1','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.1'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.2') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.2','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.2'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.3') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.3','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.3'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.4') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.4','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.4'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.5') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.5','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.5'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.6') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.6','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.6'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.7') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.7','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.7'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.8') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.8','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.8'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.1.9') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.1.9','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.1.9'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.0') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.0','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.0'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.1') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.1','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.1'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.2') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.2','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.2'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.3') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.3','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.3'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.4') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.4','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.4'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.5') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.5','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.5'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.6') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.6','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.6'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.7') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.7','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.7'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.8') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.8','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.8'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.2.9') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.2.9','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.2.9'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.0') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.0','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.0'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.1') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.1','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.1'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.2') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.2','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.2'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.3') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.3','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.3'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.4') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.4','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.4'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.5') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.5','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.5'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.6') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.6','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.6'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.7') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.7','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.7'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.8') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.8','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.8'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.3.9') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.3.9','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.3.9'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.0') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.0','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.0'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.1') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.1','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.1'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.2') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.2','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.2'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.3') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.3','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.3'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.4') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.4','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.4'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.5') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.5','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.5'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.6') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.6','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.6'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.7') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.7','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.7'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.8') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.8','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.8'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.4.9') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.4.9','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.4.9'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.0') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.0','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.5.0'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.1') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.1','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.5.1'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.2') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.2','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.5.2'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.3') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.3','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.5.3'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.4') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.4','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.5.4'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.5') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.5','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.5.5'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.6') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.6','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.5.6'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.7') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.7','Outdated','','5/25/2005 5:22:41 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Outdated',[UpdateURL] = '',[CreatedDate] = '5/25/2005 5:22:41 PM' WHERE [Version] = '1.0.5.7'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.8') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.8','Autoupdate','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','1/30/2005 2:12:00 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Autoupdate',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '1/30/2005 2:12:00 PM' WHERE [Version] = '1.0.5.8'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.5.9') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.5.9','Autoupdate','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','1/30/2005 2:12:00 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Autoupdate',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '1/30/2005 2:12:00 PM' WHERE [Version] = '1.0.5.9'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.6.0') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.6.0','Autoupdate','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','1/30/2005 2:12:00 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Autoupdate',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '1/30/2005 2:12:00 PM' WHERE [Version] = '1.0.6.0'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.6.1') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.6.1','Autoupdate','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','1/30/2005 2:12:00 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Autoupdate',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '1/30/2005 2:12:00 PM' WHERE [Version] = '1.0.6.1'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.6.2') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.6.2','Autoupdate','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','8/13/2006 1:37:13 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Autoupdate',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '8/13/2006 1:37:13 PM' WHERE [Version] = '1.0.6.2'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.6.3') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.6.3','Autoupdate','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','12/1/2006 11:49:23 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Autoupdate',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '12/1/2006 11:49:23 PM' WHERE [Version] = '1.0.6.3'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.6.4') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.6.4','Autoupdate','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','12/7/2006 1:23:39 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Autoupdate',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '12/7/2006 1:23:39 PM' WHERE [Version] = '1.0.6.4'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.6.5') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.6.5','Current','Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe','12/7/2006 1:44:20 PM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Current',[UpdateURL] = 'Http://SQLExch.Com/Updates/1.0.6.6_<Arch>.exe',[CreatedDate] = '12/7/2006 1:44:20 PM' WHERE [Version] = '1.0.6.5'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[ClientVersions] WHERE [Version] = '1.0.6.6') BEGIN
INSERT INTO [dbo].[ClientVersions] ([Version],[Status],[UpdateURL],[CreatedDate])
VALUES ('1.0.6.6','Current',null,'11/1/2007 10:34:56 AM')
END ELSE BEGIN
UPDATE [dbo].[ClientVersions] SET [Status] = 'Current',[UpdateURL] = null,[CreatedDate] = '11/1/2007 10:34:56 AM' WHERE [Version] = '1.0.6.6'
END

GO

ALTER TABLE [dbo].[ClientVersions] CHECK CONSTRAINT ALL
GO


GO
ALTER TABLE [dbo].[Company] NOCHECK CONSTRAINT ALL
GO
IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[Company] WHERE [ID] = '23') BEGIN
SET IDENTITY_INSERT [dbo].[Company] ON
INSERT INTO [dbo].[Company] ([ID],[Name],[Password],[DatabasePrefix],[InitializeStep],[MonitorWaitTime],[AverageWaitTime],[LastConnectDate],[LastKnownIPAddress],[OfflineProcessingDatabase],[OfflineProcessingSchema],[OfflineProcessingTable],[PricingDiscount],[APIAuthenticationString],[CreatedDate],[Comment],[Active])
VALUES (23,'Developmentx86','Development','DEVx86',0,0,null,'7/14/2009 4:18:20 PM','127.0.0.1',null,null,null,100.000,'31fbdf1b-c8b2-41f3-b81b-f84ccc32dac4','5/25/2005 5:20:01 PM','Development user for testing.',1)
SET IDENTITY_INSERT [dbo].[Company] OFF
END ELSE BEGIN
UPDATE [dbo].[Company] SET [Name] = 'Developmentx86',[Password] = 'Development',[DatabasePrefix] = 'DEVx86',[InitializeStep] = 0,[MonitorWaitTime] = 0,[AverageWaitTime] = null,[LastConnectDate] = '7/14/2009 4:18:20 PM',[LastKnownIPAddress] = '127.0.0.1',[OfflineProcessingDatabase] = null,[OfflineProcessingSchema] = null,[OfflineProcessingTable] = null,[PricingDiscount] = 100.000,[APIAuthenticationString] = '31fbdf1b-c8b2-41f3-b81b-f84ccc32dac4',[CreatedDate] = '5/25/2005 5:20:01 PM',[Comment] = 'Development user for testing.',[Active] = 1 WHERE [ID] = '23'
END

GO

IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[Company] WHERE [ID] = '24') BEGIN
SET IDENTITY_INSERT [dbo].[Company] ON
INSERT INTO [dbo].[Company] ([ID],[Name],[Password],[DatabasePrefix],[InitializeStep],[MonitorWaitTime],[AverageWaitTime],[LastConnectDate],[LastKnownIPAddress],[OfflineProcessingDatabase],[OfflineProcessingSchema],[OfflineProcessingTable],[PricingDiscount],[APIAuthenticationString],[CreatedDate],[Comment],[Active])
VALUES (24,'Developmentx64','Development','DEVx64',0,0,null,'7/14/2009 4:17:22 PM','127.0.0.1',null,null,null,100.000,'31fbdf1b-c8b2-41f3-b81b-f84ccc32dac4','5/25/2005 5:20:01 PM','Development user for testing.',1)
SET IDENTITY_INSERT [dbo].[Company] OFF
END ELSE BEGIN
UPDATE [dbo].[Company] SET [Name] = 'Developmentx64',[Password] = 'Development',[DatabasePrefix] = 'DEVx64',[InitializeStep] = 0,[MonitorWaitTime] = 0,[AverageWaitTime] = null,[LastConnectDate] = '7/14/2009 4:17:22 PM',[LastKnownIPAddress] = '127.0.0.1',[OfflineProcessingDatabase] = null,[OfflineProcessingSchema] = null,[OfflineProcessingTable] = null,[PricingDiscount] = 100.000,[APIAuthenticationString] = '31fbdf1b-c8b2-41f3-b81b-f84ccc32dac4',[CreatedDate] = '5/25/2005 5:20:01 PM',[Comment] = 'Development user for testing.',[Active] = 1 WHERE [ID] = '24'
END

GO

ALTER TABLE [dbo].[Company] CHECK CONSTRAINT ALL
GO


GO
ALTER TABLE [dbo].[CompanySchedules] NOCHECK CONSTRAINT ALL
GO
IF NOT EXISTS (SELECT TOP 1 1 FROM [dbo].[CompanySchedules] WHERE [ID] = '1') BEGIN
SET IDENTITY_INSERT [dbo].[CompanySchedules] ON
INSERT INTO [dbo].[CompanySchedules] ([ID],[CompanyID],[Sunday],[Monday],[Tuesday],[Wednesday],[Thursday],[Friday],[Saturday],[StartTime],[EndTime],[Increment],[Enforce],[AsOfDate],[Active])
VALUES (1,23,0,1,1,1,1,1,0,'9:00','17:00',10,0,'11/9/2007 10:29:01 AM',1)
SET IDENTITY_INSERT [dbo].[CompanySchedules] OFF
END ELSE BEGIN
UPDATE [dbo].[CompanySchedules] SET [CompanyID] = 23,[Sunday] = 0,[Monday] = 1,[Tuesday] = 1,[Wednesday] = 1,[Thursday] = 1,[Friday] = 1,[Saturday] = 0,[StartTime] = '9:00',[EndTime] = '17:00',[Increment] = 10,[Enforce] = 0,[AsOfDate] = '11/9/2007 10:29:01 AM',[Active] = 1 WHERE [ID] = '1'
END

GO

ALTER TABLE [dbo].[CompanySchedules] CHECK CONSTRAINT ALL
GO


GO
