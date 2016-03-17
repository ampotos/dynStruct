% include header
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>
<script type="text/javascript">
  $(document).ready(function() {
    table = $('#members').DataTable( {
      "processing": true,
      "ajax": {
        "url": "member_get?id_struct={{id_struct}}"
      },
  })
  table.columns().every( function () {
    var that = this;
    $( 'input', this.footer() ).on( 'keyup change', function () {
      if ( that.search() !== this.value ) {
        that
            .search( this.value )
            .draw();}})})
  });
</script>

<table id="members" class="table table-bordered" style="width: 100%">
    <thead>
      <tr>
	<th> offset </th>
	<th> detailed view </th>
	<th> size </th>
	<th> type </th>
	<th> </th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> <input class="form-control" type="text" placeholder="Search offset" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search detailed view" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search size" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search type" /> </th>
	<th> </th>
      </tr>
    </tfoot>
  </table>
% include footer
