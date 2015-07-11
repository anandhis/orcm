#
# Copyright (c) 2015 Intel Inc. All rights reserved
#
import os
from sqlalchemy import create_engine, DDL


db_engine = None


def connect_to_db(db_url, debug=False):
    """Set the db_engine instance

    :param db_url: The database connection string in the format
    postgresql[+<driver>://[<username>[:<password>]]@<server>[:<port>]/<database>.

    See SQLAlchemy database dialect for more information.
    :param debug: Set to True to enable echo of actual SQL statement being
    issued to the database
    :return:  Database engine
    """
    return create_engine(db_url, echo=debug)


def enable_partition_trigger(table_name, column_name, interval,
                             interval_to_keep, enable_purging=False):
    """Execute the generated code from generate_partition_triggers_ddl()
    function.

    :param table_name: The name of the table to apply the partition
    :param column_name: The column name to partition on.  This column need to
    be a Postgres timestamp datatype
    :param interval: The interval unit supported by Postgres (e.g. YEAR, MONTH,
    DAY, HOUR, MINUTE).  The size of the partition is one unit of this interval.
    :param interval_to_keep: The number of interval to keep before purging
    :param enable_purging: True to drop old partition when create new partition.
    :return:  None.  Only print output.
    """
    generated_ddl_sql_stmt = (
        """SELECT generate_partition_triggers_ddl('%s', '%s', '%s', %d);""" %
        (table_name, column_name, interval, interval_to_keep))
    print(generated_ddl_sql_stmt)
    partition_trigger_ddl_stmt = db_engine.execute(generated_ddl_sql_stmt).scalar()

    # Enable auto purging within the table partitioning function
    if enable_purging:
        partition_trigger_ddl_stmt = partition_trigger_ddl_stmt.replace(
            "-- EXECUTE('DROP TABLE IF EXISTS ' || quote_ident('",
            "EXECUTE('DROP TABLE IF EXISTS ' || quote_ident('")
        print("Uncommented the DROP expired partition as new partition being "
              "created")
    print(partition_trigger_ddl_stmt)
    db_engine.execute(DDL(partition_trigger_ddl_stmt))
    print("Partition trigger enabled for table '%s' on column '%s' "
          "split every 1 %s and %s" % (
        table_name, column_name, interval,
        "keep only last %d intervals" % (interval_to_keep) if enable_purging
        else "keep all intervals"))


def disable_partition_trigger(table_name):
    """Drop the trigger if exists that having the name
    "insert_<table_name>_trigger" on <table_name>.

    Also drop the function if exists that having the name
    "<table_name>_partition_handler()".

     Warning:  This function is not aware of the contents of the trigger and
     function named above.  The names are the expected format generated by the
     generate_partition_triggers_ddl() function.

    :param table_name: The name of the table to construct the name of the
    insert trigger and partition handler.
    :return:  None.  Only print output.
    """
    drop_trigger_ddl_stmt = (
        "DROP TRIGGER IF EXISTS insert_%s_trigger ON %s RESTRICT;" %
        (table_name, table_name))
    print(drop_trigger_ddl_stmt)
    db_engine.execute(DDL(drop_trigger_ddl_stmt))
    drop_function_ddl_stmt = (
        "DROP FUNCTION IF EXISTS %s_partition_handler() RESTRICT;" % table_name)
    print(drop_function_ddl_stmt)
    db_engine.execute(DDL(drop_function_ddl_stmt))

    print("Partition trigger disabled for table '%s.'  Existing partitions left "
          "as is." % table_name)


def main():
    """Main driver"""
    table_name = "data_sample_raw"
    column_name = "time_stamp"
    interval = "DAY"
    interval_to_keep = 10
    enable_purging = True

    disable_partition_trigger(table_name)

    enable_partition_trigger(table_name, column_name, interval,
                             interval_to_keep, enable_purging=enable_purging)


if __name__ == "__main__":
    db_url = os.getenv("PG_DB_URL")
    if not db_url:
        raise RuntimeError ("The 'PG_DB_URL' environment variable is not set.  "
              "Please set this environment variable with the db_url with the "
              "following patter:\n"
              "postgresql[+<driver>://[<username>[:<password>]]@<server>"
              "[:<port>]/<database>")

    db_engine = connect_to_db(db_url=db_url)
    main()
