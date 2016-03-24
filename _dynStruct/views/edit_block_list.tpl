% include header
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>

<script type="text/javascript">
  $(document).ready(function() {
  instance_table = $('#instance_blocks').DataTable( {
  "processing": true,
  "ajax": {
  "url": "struct_instance_get?id_struct={{id_struct}}&instance=true"
  },
  "columnDefs": [
  { "visible": false, "targets": 5 }
  ]
  })

  potential_table = $('#potential_blocks').DataTable( {
  "processing": true,
  "ajax": {
  "url": "struct_instance_get?id_struct={{id_struct}}&instance=false"
  // remove last element of each row (== block id)
  },
  "columnDefs": [
  { "visible": false, "targets": 5 }
  ]
  })

  instance_table.columns().every( function () {
  var that = this;
  $( 'input', this.footer() ).on( 'keyup change', function () {
  if ( that.search() !== this.value ) {
  that
  .search( this.value )
  .draw();}})})

  potential_table.columns().every( function () {
  var that = this;
  $( 'input', this.footer() ).on( 'keyup change', function () {
  if ( that.search() !== this.value ) {
  that
  .search( this.value )
  .draw();}})})


  remove_list = []
  $('#instance_blocks').on( 'click', 'tr', function () {
  $(this).toggleClass('active');
  block_id = $("#instance_blocks").dataTable().fnGetData($(this))[5];
  idx = $.inArray(block_id, remove_list)
  if (idx < 0) {
     remove_list.push(block_id)
  }
  else {
     remove_list.splice(idx, 1);
  }
  } );

  add_list = []
  $('#potential_blocks tbody').on( 'click', 'tr', function () {
  $(this).toggleClass('active');

  block_id = $("#potential_blocks").dataTable().fnGetData($(this))[5];
  idx = $.inArray(block_id, add_list)
  if (idx < 0) {
     add_list.push(block_id)
  }
  else {
     add_list.splice(idx, 1);
  }
  } );

  $('#edit_instances').click( function () {
      var xhr = new XMLHttpRequest();
      data = "add=" + add_list.join(',') + "&remove=" + remove_list.join(',');

      xhr.open("POST", "/struct_instance_do_edit?id={{id_struct}}", true);
      xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      xhr.onloadend = function() {
      window.location.replace("/struct?id={{id_struct}}");
      }
      xhr.send(data);
  } );
});
</script>

<div class="container-fluid">
  <div class="text-center">
    <input id="edit_instances" class="btn btn-primary" type="submit" value="Edit instances" />
  </div>

  <h1 class="text-center"> Actual instances </h1>
  <h3 class="text-center"> Select to remove blocks from instances of {{struct_name}} </h3>

  <table id="instance_blocks" class="table table-bordered" style="width: 100%">
    <thead>
      <tr>
	<th>start</th>
	<th>size</th>
	<th>malloc caller</th>
	<th>free caller</th>
	<th>detailed view</th>
	<th>id</th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> <input class="form-control" type="text" placeholder="Search adress" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search size" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search malloc caller" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search free caller" /> </th>
	<th>  </th>
	<th>  </th>
      </tr>
    </tfoot>
  </table>

  <h1 class="text-center"> Potential instances </h1>
  <h3 class="text-center"> Select to add blocks as instance of {{struct_name}} </h3>

  <table id="potential_blocks" class="table table-bordered" style="width: 100%">
    <thead>
      <tr>
	<th>start</th>
	<th>size</th>
	<th>malloc caller</th>
	<th>free caller</th>
	<th>detailed view</th>
	<th>id</th>
      </tr>
    </thead>
    <tfoot>
      <tr>
	<th> <input class="form-control" type="text" placeholder="Search adress" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search size" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search malloc caller" /> </th>
	<th> <input class="form-control" type="text" placeholder="Search free caller" /> </th>
	<th>  </th>
	<th>  </th>
      </tr>
    </tfoot>
  </table>
</div>
% include footer
